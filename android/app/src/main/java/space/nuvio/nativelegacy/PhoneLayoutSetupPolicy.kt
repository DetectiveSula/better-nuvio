package space.nuvio.nativelegacy

/** A local first-run explanation; never overrides a saved TV/Mobile choice. */
object PhoneLayoutSetupPolicy {
    fun shouldShow(detectedMobile: Boolean, mobileInterface: Boolean, completed: Boolean): Boolean =
        detectedMobile && mobileInterface && !completed
}
