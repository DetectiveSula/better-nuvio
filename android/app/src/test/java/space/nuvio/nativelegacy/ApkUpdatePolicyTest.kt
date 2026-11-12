package space.nuvio.nativelegacy

import org.junit.Assert.*
import org.junit.Test

class ApkUpdatePolicyTest {
    private val pkg = "space.nuvio.nativelegacy.strictdebug"
    private fun accepts(name: String? = pkg, version: Long = 20009,
                        installed: Set<String> = setOf("signer-a"),
                        incoming: Set<String> = setOf("signer-a")) =
        ApkUpdatePolicy.compatible(pkg, 20008, installed, name, version, incoming)

    @Test fun newerSamePackageAndSignerIsAccepted() { assertTrue(accepts()) }
    @Test fun productionAndOtherPackagesAreRejected() {
        assertFalse(accepts("space.nuvio.nativelegacy"))
        assertFalse(accepts("org.example.other")); assertFalse(accepts(null))
    }
    @Test fun equalAndOlderVersionsAreRejected() {
        assertFalse(accepts(version = 20008)); assertFalse(accepts(version = 20007))
    }
    @Test fun missingOrChangedSignersAreRejected() {
        assertFalse(accepts(installed = emptySet())); assertFalse(accepts(incoming = emptySet()))
        assertFalse(accepts(incoming = setOf("signer-b")))
        assertFalse(accepts(incoming = setOf("signer-a", "signer-b")))
    }
    @Test fun multipleSignersRequireExactSet() {
        assertTrue(accepts(installed = setOf("a", "b"), incoming = setOf("b", "a")))
    }
}
