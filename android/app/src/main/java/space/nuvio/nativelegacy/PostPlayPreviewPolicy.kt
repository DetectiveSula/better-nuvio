package space.nuvio.nativelegacy

import java.net.URI

internal object PostPlayPreviewPolicy {
    fun softwareAvc(name: String, mime: String, secure: Boolean = false, tunneled: Boolean = false): Boolean =
        !secure && !tunneled && mime.equals("video/avc", true) &&
            (name.startsWith("OMX.google.") || name.startsWith("c2.android."))
    fun supported(lowRam: Boolean, softwareAvc: Boolean): Boolean = !lowRam && softwareAvc
    fun safeUrl(url: String): Boolean = try {
        val uri = URI(url)
        uri.scheme == "https" && !uri.host.isNullOrEmpty() && uri.rawUserInfo == null
    } catch (_: Exception) { false }
}
