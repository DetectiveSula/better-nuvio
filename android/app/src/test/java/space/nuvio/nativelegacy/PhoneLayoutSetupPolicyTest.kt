package space.nuvio.nativelegacy

import org.junit.Assert.assertEquals
import org.junit.Test

class PhoneLayoutSetupPolicyTest {
    @Test fun setupOnlyForUnacknowledgedMobileDevices() {
        for (detected in listOf(false, true)) for (mobile in listOf(false, true))
            for (completed in listOf(false, true)) {
                assertEquals(detected && mobile && !completed,
                    PhoneLayoutSetupPolicy.shouldShow(detected, mobile, completed))
            }
    }
}
