package space.nuvio.nativelegacy

import org.junit.Assert.*
import org.junit.Test

class PostPlayPreviewPolicyTest {
    @Test fun neverUsesHardwareOrProtectedDecoders() {
        assertTrue(PostPlayPreviewPolicy.softwareAvc("c2.android.avc.decoder", "video/avc"))
        assertTrue(PostPlayPreviewPolicy.softwareAvc("OMX.google.h264.decoder", "video/avc"))
        for (name in listOf("OMX.qcom.video.decoder.avc", "c2.mtk.avc.decoder", "OMX.amlogic.avc.decoder"))
            assertFalse(PostPlayPreviewPolicy.softwareAvc(name, "video/avc"))
        assertFalse(PostPlayPreviewPolicy.softwareAvc("c2.android.hevc.decoder", "video/hevc"))
        assertFalse(PostPlayPreviewPolicy.softwareAvc("c2.android.avc.decoder", "video/avc", secure = true))
        assertFalse(PostPlayPreviewPolicy.softwareAvc("c2.android.avc.decoder", "video/avc", tunneled = true))
    }
    @Test fun lowMemoryDevicesKeepPosters() {
        assertTrue(PostPlayPreviewPolicy.supported(false, true))
        assertFalse(PostPlayPreviewPolicy.supported(true, true))
        assertFalse(PostPlayPreviewPolicy.supported(false, false))
        assertFalse(PostPlayPreviewPolicy.supported(true, false))
    }
    @Test fun previewUrlsCannotCarryLoginCredentials() {
        assertTrue(PostPlayPreviewPolicy.safeUrl("https://fixture.invalid/trailer.mp4?Expires=123"))
        for (url in listOf("http://fixture.invalid/x", "file:///x", "https://user:fixture@fixture.invalid/x", "https:///x", "invalid"))
            assertFalse(PostPlayPreviewPolicy.safeUrl(url))
    }
}
