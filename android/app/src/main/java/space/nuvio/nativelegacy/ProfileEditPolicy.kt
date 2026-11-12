package space.nuvio.nativelegacy

/** Account names also have to fit the native UTF-8 cache. */
object ProfileEditPolicy {
    const val NAME_MAX_CHARACTERS = 20
    const val NAME_MAX_BYTES = 63
    const val PIN_LENGTH = 4

    /** Official Nuvio TV's profile colours (domain/model/ProfileAvatarColors.kt), in its order. */
    val COLORS = listOf("#E53935", "#1E88E5", "#8E24AA", "#43A047", "#FFB300", "#D81B60", "#00ACC1", "#6D4C41")

    fun validName(value: String): Boolean = value.trim().let {
        it.isNotEmpty() && it.codePointCount(0, it.length) <= NAME_MAX_CHARACTERS &&
            it.toByteArray(Charsets.UTF_8).size <= NAME_MAX_BYTES && it.none { c -> c.isISOControl() }
    }

    /**
     * What the name box keeps when [incoming] would replace [current]: the typed text when it
     * still fits the account limits, otherwise the longest prefix of it that does (never a
     * split surrogate pair). Typing past the limit simply stops, instead of failing at Save.
     */
    fun clampName(incoming: String): String {
        val clean = incoming.filterNot { it.isISOControl() }
        var end = 0; var count = 0; var bytes = 0
        while (end < clean.length) {
            val cp = clean.codePointAt(end); val width = Character.charCount(cp)
            val size = String(Character.toChars(cp)).toByteArray(Charsets.UTF_8).size
            if (count + 1 > NAME_MAX_CHARACTERS || bytes + size > NAME_MAX_BYTES) break
            count++; bytes += size; end += width
        }
        return clean.substring(0, end)
    }

    fun validPhotoUrl(value: String): Boolean = value.isEmpty() ||
        ((value.startsWith("https://") || value.startsWith("http://")) &&
            value.toByteArray(Charsets.UTF_8).size < 300 && value.none { it.isWhitespace() || it.isISOControl() })

    /** `#RRGGBB` only: it is stored in a 10-byte native field and parsed by Color.parseColor. */
    fun validColor(value: String): Boolean = Regex("#[0-9A-Fa-f]{6}").matches(value)

    /** Catalogue colours sometimes arrive without '#', or as #AARRGGBB. */
    fun normalColor(value: String?): String? {
        val v = value?.trim().orEmpty().removePrefix("#")
        val rgb = when (v.length) { 6 -> v; 8 -> v.substring(2); else -> return null }
        return "#$rgb".uppercase().takeIf { validColor(it) }
    }

    fun validPin(value: String): Boolean = value.length == PIN_LENGTH && value.all { it in '0'..'9' }

    /**
     * Profiles this editor may delete: never profile 1 or the primary profile (official Nuvio
     * refuses both), and never the profile the app is currently running as.
     */
    fun canDelete(profile: Int, primary: Boolean, active: Int): Boolean = profile > 1 && !primary && profile != active
}
