package space.nuvio.nativelegacy

import org.json.JSONArray
import org.json.JSONObject
import java.net.HttpURLConnection
import java.net.URL
import java.util.UUID

/** Sparse edits only. Credentials are held in memory and never logged. */
internal class ProfileAccountApi(private val config: JSONObject, private val current: () -> Boolean = { true }) {
    data class Avatar(val id: String, val name: String, val url: String, val color: String? = null)
    /** A supporter background from the member catalogue; its image needs an authenticated read. */
    data class Background(val id: String, val name: String, val path: String)
    /** Background to save: both null = the standard (poster wall) background. */
    data class BackgroundChoice(val id: String?, val url: String?)
    data class Saved(val name: String, val photo: String, val color: String?, val background: String?)
    private val base = config.getString("base").trimEnd('/')
    private val token = config.getString("token")
    private val key = config.getString("key")
    private val readOnly = config.getBoolean("readonly")
    private val profile = config.getInt("profile")
    private val user = config.getString("user")
    private val client = config.optString("client")

    private fun get(path: String, max: Int): ByteArray {
        check(current()) { "This profile editor has closed." }
        val connection = URL(base + path).openConnection() as HttpURLConnection
        connection.instanceFollowRedirects = false
        connection.connectTimeout = 15_000; connection.readTimeout = 25_000
        connection.setRequestProperty("apikey", key)
        connection.setRequestProperty("Authorization", "Bearer $token")
        try {
            check(connection.responseCode in 200..299) { "Could not load this image." }
            return connection.inputStream.use { it.readNBytesCompat(max) }
        } finally { connection.disconnect() }
    }

    private fun request(path: String, body: ByteArray, type: String = "application/json", write: Boolean): String {
        check(current()) { "This profile editor has closed." }
        if (write && readOnly) error("Account saves are disabled in this read-only test build.")
        val connection = URL(base + path).openConnection() as HttpURLConnection
        connection.instanceFollowRedirects = false
        connection.requestMethod = "POST"
        connection.connectTimeout = 15_000
        connection.readTimeout = 25_000
        connection.doOutput = true
        connection.setRequestProperty("apikey", key)
        connection.setRequestProperty("Authorization", "Bearer $token")
        connection.setRequestProperty("Content-Type", type)
        connection.setFixedLengthStreamingMode(body.size)
        try {
            check(current()) { "This profile editor has closed." }
            connection.outputStream.use { it.write(body) }
            val status = connection.responseCode
            if (status !in 200..299) {
                // Do not expose response bodies, tokens or signed image URLs.
                throw IllegalStateException(when {
                    status == 401 -> "Your session expired. Sign in again before saving."
                    path.contains("profile_pin") && status in 400..403 -> "The current PIN was not accepted. Try again."
                    path.startsWith("/storage/") && status == 403 -> "This account server does not allow personal photo uploads. Choose a Nuvio avatar or use a photo URL."
                    status == 404 -> "This account server does not support this profile action."
                    else -> "Could not save your changes (HTTP $status). Try again."
                })
            }
            return connection.inputStream.use { input ->
                val data = input.readNBytesCompat(1024 * 1024)
                String(data, Charsets.UTF_8)
            }
        } finally { connection.disconnect() }
    }

    fun avatars(): List<Avatar> {
        val rows = JSONArray(request("/rest/v1/rpc/get_avatar_catalog", "{}".toByteArray(), write = false))
        return (0 until minOf(rows.length(), 64)).mapNotNull {
            val row = rows.optJSONObject(it) ?: return@mapNotNull null
            val id = row.optString("id")
            val path = row.optString("storage_path").trimStart('/')
            if (id.isEmpty() || id.length >= 64 || path.isEmpty() || path.contains("..") || path.contains(":") || path.contains("?")) return@mapNotNull null
            Avatar(id, row.optString("display_name", id.removePrefix("avatar_").replace('_', ' ')),
                "$base/storage/v1/object/public/avatars/$path",
                ProfileEditPolicy.normalColor(if (row.isNull("bg_color")) null else row.optString("bg_color")))
        }
    }

    /** POST a new immutable object; never overwrite another profile's photo. */
    fun upload(photo: ByteArray): String {
        require(photo.size in 1..(1024 * 1024))
        require(user.matches(Regex("[a-zA-Z0-9-]{1,80}")))
        val path = "$user/profile-$profile-${UUID.randomUUID()}.jpg"
        val url = "$base/storage/v1/object/public/avatars/$path"
        require(ProfileEditPolicy.validPhotoUrl(url))
        request("/storage/v1/object/avatars/$path", photo, "image/jpeg", write = true)
        return url
    }

    /** Supporter backgrounds; accounts without membership get an error, which callers treat as "none". */
    fun backgrounds(): List<Background> {
        val rows = JSONArray(request("/rest/v1/rpc/get_member_profile_background_catalog", "{}".toByteArray(), write = false))
        return (0 until minOf(rows.length(), 64)).mapNotNull {
            val row = rows.optJSONObject(it) ?: return@mapNotNull null
            val id = row.optString("id"); val path = row.optString("storage_path").trimStart('/')
            if (id.isEmpty() || id.length >= 64 || !safePath(path)) return@mapNotNull null
            Background(id, row.optString("display_name", id), path)
        }
    }
    fun backgroundImage(background: Background): ByteArray {
        require(safePath(background.path))
        return get("/storage/v1/object/authenticated/membership-profile-backgrounds/${background.path}", 4 * 1024 * 1024)
    }
    private fun safePath(path: String) = path.isNotEmpty() && path.length < 200 &&
        !path.contains("..") && !path.contains(":") && !path.contains("?") && !path.contains("#")

    /**
     * Name and photo alone use the sparse `sync_patch_profile`. Colour and background are not
     * part of that call, so a change to either goes the way official Nuvio saves profiles:
     * `sync_push_profiles` with every profile, where every other profile is copied unchanged
     * from a `sync_pull_profiles` read made just before the write.
     */
    fun save(name: String, changedPhoto: Boolean, avatar: Avatar?, photoUrl: String,
             color: String? = null, background: BackgroundChoice? = null): Saved {
        require(ProfileEditPolicy.validName(name))
        require(ProfileEditPolicy.validPhotoUrl(photoUrl))
        require(color == null || ProfileEditPolicy.validColor(color))
        require(background?.url == null || (background.url.isNotEmpty() && ProfileEditPolicy.validPhotoUrl(background.url)))
        if (color != null || background != null) return saveAll(name.trim(), changedPhoto, avatar, photoUrl, color, background)
        val body = JSONObject().put("p_profile_id", profile).put("p_name", name.trim())
        if (changedPhoto) {
            body.put("p_avatar_id_provided", true).put("p_avatar_url_provided", true)
                .put("p_avatar_id", avatar?.id ?: JSONObject.NULL)
                .put("p_avatar_url", if (avatar == null && photoUrl.isNotEmpty()) photoUrl else JSONObject.NULL)
        }
        val response = request("/rest/v1/rpc/sync_patch_profile", body.toString().toByteArray(), write = true)
        val row = if (response.trimStart().startsWith("[")) JSONArray(response).optJSONObject(0) else JSONObject(response)
        require(row != null && row.optInt("profile_index") == profile) { "The account server did not confirm this profile. Try again." }
        val savedName = row.optString("name", name.trim())
        val savedPhoto = if (!row.isNull("avatar_url")) row.optString("avatar_url")
            else avatar?.url ?: if (!changedPhoto) config.optString("photo") else ""
        require(ProfileEditPolicy.validName(savedName) && ProfileEditPolicy.validPhotoUrl(savedPhoto))
        return Saved(savedName, savedPhoto, null, null)
    }

    private fun pullProfiles(): List<JSONObject> {
        val rows = JSONArray(request("/rest/v1/rpc/sync_pull_profiles", "{}".toByteArray(), write = false))
        val list = (0 until rows.length()).map { rows.optJSONObject(it) ?: error("The account returned an unreadable profile list.") }
        val ids = list.map { it.optInt("profile_index", 0) }
        require(list.size <= MAX_PROFILES && ids.all { it in 1..8 } && ids.toSet().size == ids.size) {
            "The account returned an unexpected profile list. Nothing was changed."
        }
        return list
    }
    /** Copies every field official Nuvio pushes, exactly as the account returned it. */
    private fun payload(row: JSONObject): JSONObject {
        fun raw(key: String): Any = if (row.has(key)) row.get(key) else JSONObject.NULL
        return JSONObject().put("profile_index", row.getInt("profile_index")).put("name", row.optString("name"))
            .put("avatar_color_hex", row.optString("avatar_color_hex", "#1E88E5").ifEmpty { "#1E88E5" })
            .put("uses_primary_addons", row.optBoolean("uses_primary_addons", false))
            .put("uses_primary_plugins", row.optBoolean("uses_primary_plugins", false))
            .put("avatar_id", raw("avatar_id")).put("avatar_url", raw("avatar_url"))
            .put("profile_background_id", raw("profile_background_id"))
            .put("profile_background_url", raw("profile_background_url"))
    }
    private fun push(profiles: List<JSONObject>) {
        val body = JSONObject().put("p_client_max_profiles", MAX_PROFILES).put("p_profiles", JSONArray(profiles))
        if (client.isNotEmpty()) body.put("p_origin_client_id", client)
        request("/rest/v1/rpc/sync_push_profiles", body.toString().toByteArray(), write = true)
    }
    private fun saveAll(name: String, changedPhoto: Boolean, avatar: Avatar?, photoUrl: String,
                        color: String?, background: BackgroundChoice?): Saved {
        val rows = pullProfiles()
        require(rows.any { it.getInt("profile_index") == profile }) { "This profile is no longer on the account." }
        push(rows.map { row ->
            val p = payload(row)
            if (row.getInt("profile_index") == profile) {
                p.put("name", name)
                if (color != null) p.put("avatar_color_hex", color)
                if (changedPhoto) p.put("avatar_id", avatar?.id ?: JSONObject.NULL)
                    .put("avatar_url", if (avatar == null && photoUrl.isNotEmpty()) photoUrl else JSONObject.NULL)
                if (background != null) p.put("profile_background_id", background.id ?: JSONObject.NULL)
                    .put("profile_background_url", background.url ?: JSONObject.NULL)
            }
            p
        })
        val row = pullProfiles().firstOrNull { it.getInt("profile_index") == profile }
            ?: error("The account server did not confirm this profile. Try again.")
        val savedName = row.optString("name", name)
        val savedPhoto = if (!row.isNull("avatar_url") && row.optString("avatar_url").isNotEmpty()) row.optString("avatar_url")
            else if (changedPhoto) avatar?.url ?: "" else config.optString("photo")
        val savedColor = ProfileEditPolicy.normalColor(row.optString("avatar_color_hex")) ?: color
        val savedBackground = if (row.isNull("profile_background_url")) "" else row.optString("profile_background_url")
        require(ProfileEditPolicy.validName(savedName) && ProfileEditPolicy.validPhotoUrl(savedPhoto) &&
            ProfileEditPolicy.validPhotoUrl(savedBackground)) { "The account server returned an unexpected profile." }
        return Saved(savedName, savedPhoto, savedColor, savedBackground)
    }

    fun setPin(pin: String, currentPin: String?) {
        require(ProfileEditPolicy.validPin(pin) && (currentPin == null || ProfileEditPolicy.validPin(currentPin)))
        val body = JSONObject().put("p_profile_id", profile).put("p_pin", pin)
        if (currentPin != null) body.put("p_current_pin", currentPin)
        request("/rest/v1/rpc/set_profile_pin", body.toString().toByteArray(), write = true)
    }
    fun clearPin(currentPin: String?) {
        require(currentPin == null || ProfileEditPolicy.validPin(currentPin))
        val body = JSONObject().put("p_profile_id", profile)
        if (currentPin != null) body.put("p_current_pin", currentPin)
        request("/rest/v1/rpc/clear_profile_pin", body.toString().toByteArray(), write = true)
    }

    /**
     * Official Nuvio TV: delete the profile's synced data, then push the list without it. The
     * push is skipped when the delete already removed the profile row (official mobile).
     */
    fun delete(primary: Boolean, active: Int) {
        require(ProfileEditPolicy.canDelete(profile, primary, active)) { "This profile can't be deleted." }
        val body = JSONObject().put("p_profile_id", profile)
        if (client.isNotEmpty()) body.put("p_origin_client_id", client)
        request("/rest/v1/rpc/sync_delete_profile_data", body.toString().toByteArray(), write = true)
        val rows = pullProfiles()
        if (rows.any { it.getInt("profile_index") == profile }) {
            require(rows.size > 1)
            push(rows.filter { it.getInt("profile_index") != profile }.map { payload(it) })
            require(pullProfiles().none { it.getInt("profile_index") == profile }) { "The account server did not remove this profile. Try again." }
        }
    }

    companion object { const val MAX_PROFILES = 6 }

    private fun java.io.InputStream.readNBytesCompat(max: Int): ByteArray {
        val output = java.io.ByteArrayOutputStream()
        val buffer = ByteArray(8192)
        while (true) {
            val count = read(buffer); if (count < 0) break
            if (output.size() + count > max) error("The account response was too large.")
            output.write(buffer, 0, count)
        }
        return output.toByteArray()
    }
}
