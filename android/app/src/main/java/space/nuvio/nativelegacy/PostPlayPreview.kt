package space.nuvio.nativelegacy

import android.app.Activity
import android.app.ActivityManager
import android.content.Context
import android.media.MediaCodecList
import android.net.Uri
import android.graphics.Matrix
import android.view.TextureView
import android.view.View
import android.widget.FrameLayout
import androidx.media3.common.C
import androidx.media3.common.MediaItem
import androidx.media3.common.PlaybackException
import androidx.media3.common.Player
import androidx.media3.common.VideoSize
import androidx.media3.common.util.UnstableApi
import androidx.media3.exoplayer.DefaultLoadControl
import androidx.media3.exoplayer.DefaultRenderersFactory
import androidx.media3.exoplayer.ExoPlayer
import androidx.media3.exoplayer.mediacodec.MediaCodecSelector
import androidx.media3.exoplayer.trackselection.DefaultTrackSelector

/** Muted, software-only promotional preview. Never calls NvPlayer or its progress callbacks. */
@androidx.annotation.OptIn(UnstableApi::class)
class PostPlayPreview(private val activity: Activity, private val layer: FrameLayout) {
    @Volatile var state = 0
        private set
    @Volatile private var foreground = false
    private var generation = 0
    private var player: ExoPlayer? = null
    private var texture: TextureView? = null
    private val softwareAvc by lazy {
        try {
            MediaCodecList(MediaCodecList.REGULAR_CODECS).codecInfos.any {
                !it.isEncoder && it.supportedTypes.any { type -> PostPlayPreviewPolicy.softwareAvc(it.name, type) }
            }
        } catch (_: Exception) { false }
    }
    fun supported(): Boolean = PostPlayPreviewPolicy.supported(
        (activity.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager).isLowRamDevice, softwareAvc)
    fun foreground(active: Boolean) {
        foreground = active
        if (!active) close()
    }
    fun close() {
        activity.runOnUiThread {
            generation++
            state = 0
            player?.release()
            player = null
            texture?.let { layer.removeView(it) }
            texture = null
        }
    }
    fun open(url: String, x: Float, y: Float, width: Float, height: Float) {
        activity.runOnUiThread {
            close()
            if (!foreground || !supported()) return@runOnUiThread
            val uri = Uri.parse(url)
            if (!PostPlayPreviewPolicy.safeUrl(url)) return@runOnUiThread
            val request = generation
            state = 1
            try {
                val view = TextureView(activity).apply {
                    isFocusable = false
                    isFocusableInTouchMode = false
                    importantForAccessibility = View.IMPORTANT_FOR_ACCESSIBILITY_NO
                    alpha = 0f
                }
                texture = view
                val sx = layer.width / 1920f
                val sy = layer.height / 1080f
                layer.addView(view, FrameLayout.LayoutParams(
                    (width * sx).toInt().coerceAtLeast(1), (height * sy).toInt().coerceAtLeast(1)
                ).apply { leftMargin = (x * sx).toInt(); topMargin = (y * sy).toInt() })
                // Never compete for the hardware decoder used by the feature film.
                val codecs = MediaCodecSelector { mime, secure, tunneled ->
                    if (secure || tunneled || mime != "video/avc") emptyList()
                    else MediaCodecSelector.DEFAULT.getDecoderInfos(mime, false, false).filter {
                        PostPlayPreviewPolicy.softwareAvc(it.name, mime, secure, tunneled)
                    }
                }
                val tracks = DefaultTrackSelector(activity).apply {
                    setParameters(buildUponParameters()
                        .setTrackTypeDisabled(C.TRACK_TYPE_AUDIO, true)
                        .setTrackTypeDisabled(C.TRACK_TYPE_TEXT, true))
                }
                val p = ExoPlayer.Builder(activity, DefaultRenderersFactory(activity).setMediaCodecSelector(codecs))
                    .setReleaseTimeoutMs(200)
                    .setDetachSurfaceTimeoutMs(200)
                    .setTrackSelector(tracks)
                    .setLoadControl(DefaultLoadControl.Builder().setBufferDurationsMs(500, 2000, 250, 500).build())
                    .build()
                player = p
                p.volume = 0f
                p.setVideoTextureView(view)
                p.addListener(object : Player.Listener {
                    override fun onVideoSizeChanged(videoSize: VideoSize) {
                        if (request != generation || videoSize.height <= 0 || view.height <= 0) return
                        val source = videoSize.width * videoSize.pixelWidthHeightRatio / videoSize.height
                        val target = view.width.toFloat() / view.height
                        if (source > 0 && target > 0) view.setTransform(Matrix().apply {
                            setScale(if (source > target) source / target else 1f,
                                if (source < target) target / source else 1f, view.width / 2f, view.height / 2f)
                        })
                    }
                    override fun onRenderedFirstFrame() {
                        if (request == generation && foreground) { state = 2; view.alpha = 1f }
                    }
                    override fun onPlayerError(error: PlaybackException) {
                        if (request == generation) { close(); state = -1 }
                    }
                    override fun onPlaybackStateChanged(playbackState: Int) {
                        if (request == generation && playbackState == Player.STATE_ENDED) { close(); state = -1 }
                    }
                })
                p.setMediaItem(MediaItem.fromUri(uri))
                p.prepare()
                p.playWhenReady = true
            } catch (_: Exception) { close(); state = -1 }
        }
    }
}
