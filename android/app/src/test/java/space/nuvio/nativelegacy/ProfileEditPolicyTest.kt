package space.nuvio.nativelegacy
import org.junit.Assert.*
import org.junit.Test
class ProfileEditPolicyTest {
    @Test fun namesRespectCloudAndUtf8CacheLimits() {
        assertTrue(ProfileEditPolicy.validName("  Zoë  "))
        assertFalse(ProfileEditPolicy.validName(" "))
        assertFalse(ProfileEditPolicy.validName("x".repeat(21)))
        assertTrue(ProfileEditPolicy.validName("😀".repeat(15)))
        assertFalse(ProfileEditPolicy.validName("😀".repeat(16)))
        assertFalse(ProfileEditPolicy.validName("Jo\nSmith"))
    }
    @Test fun photoUrlsNeverAcceptLocalFilesOrTruncate() {
        assertTrue(ProfileEditPolicy.validPhotoUrl("https://example.com/me.jpg"))
        assertTrue(ProfileEditPolicy.validPhotoUrl(""))
        assertFalse(ProfileEditPolicy.validPhotoUrl("file:///private/photo.jpg"))
        assertFalse(ProfileEditPolicy.validPhotoUrl("content://photos/1"))
        assertFalse(ProfileEditPolicy.validPhotoUrl("https://example.com/" + "x".repeat(300)))
        assertFalse(ProfileEditPolicy.validPhotoUrl("https://example.com/a b.jpg"))
    }
    @Test fun typingStopsAtTheAccountLimitInsteadOfFailingAtSave() {
        assertEquals("x".repeat(20), ProfileEditPolicy.clampName("x".repeat(40)))
        assertEquals("😀".repeat(15), ProfileEditPolicy.clampName("😀".repeat(16)))   // 63-byte cap, no split pair
        assertEquals("ab", ProfileEditPolicy.clampName("a\nb"))
        assertTrue(ProfileEditPolicy.validName(ProfileEditPolicy.clampName("é".repeat(40))))
    }
    @Test fun coloursAndPins() {
        assertEquals(8, ProfileEditPolicy.COLORS.size); assertTrue(ProfileEditPolicy.COLORS.all(ProfileEditPolicy::validColor))
        assertEquals("#FF8800", ProfileEditPolicy.normalColor("ff8800")); assertEquals("#112233", ProfileEditPolicy.normalColor("#FF112233"))
        assertNull(ProfileEditPolicy.normalColor("red")); assertFalse(ProfileEditPolicy.validColor("#12345"))
        assertTrue(ProfileEditPolicy.validPin("0420")); assertFalse(ProfileEditPolicy.validPin("042")); assertFalse(ProfileEditPolicy.validPin("04a0"))
    }
    @Test fun onlySecondaryInactiveProfilesCanBeDeleted() {
        assertTrue(ProfileEditPolicy.canDelete(3, false, 1))
        assertFalse(ProfileEditPolicy.canDelete(1, false, 2)); assertFalse(ProfileEditPolicy.canDelete(3, true, 1))
        assertFalse(ProfileEditPolicy.canDelete(3, false, 3))
    }
}
