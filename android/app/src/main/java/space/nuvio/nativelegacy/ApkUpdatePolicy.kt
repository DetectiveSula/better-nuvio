package space.nuvio.nativelegacy

/** No cross-package installs, signer changes or equal/older-version recuts. */
internal object ApkUpdatePolicy {
    fun compatible(
        installedPackage: String, installedVersion: Long, installedSigners: Set<String>,
        incomingPackage: String?, incomingVersion: Long, incomingSigners: Set<String>
    ): Boolean = incomingPackage == installedPackage && incomingVersion > installedVersion &&
        installedSigners.isNotEmpty() && installedSigners == incomingSigners
}
