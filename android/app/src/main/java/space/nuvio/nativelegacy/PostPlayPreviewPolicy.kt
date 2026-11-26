package space.nuvio.nativelegacy

import java.net.URI

internal object PostPlayPreviewPolicy {
    fun softwareAvc(name: String, mime: String, secure: Boolean = false, tunneled: Boolean = false): Boolean =
        !secure && !tunneled && mime.equals("video/avc", true) &&
            (name.startsWith("OMX.google.") || name.startsWith("c2.android."))
    fun supported(lowRam: Boolean, softwareAvc: Boolean): Boolean = !lowRam && softwareAvc
    /** https anywhere; since 26/11/2026 also the trailers the app itself serves:
     *  Apple's reduced HLS master as a file under one of `appDirs` (the app's own
     *  cache/files folders) and YouTube's reduced playlist on the local server
     *  (http://127.0.0.1). Never credentials, never another file or host. */
    fun safeUrl(url: String, appDirs: List<String> = emptyList()): Boolean = try {
        val uri = URI(url)
        when (uri.scheme) {
            "https" -> !uri.host.isNullOrEmpty() && uri.rawUserInfo == null
            "http" -> uri.host == "127.0.0.1" && uri.port > 0 && uri.rawUserInfo == null
            "file" -> {
                val path = uri.path ?: ""
                !path.contains("/../") && !path.endsWith("/..") &&
                    appDirs.any { it.isNotEmpty() && path.startsWith(it.trimEnd('/') + "/") }
            }
            else -> false
        }
    } catch (_: Exception) { false }
}
