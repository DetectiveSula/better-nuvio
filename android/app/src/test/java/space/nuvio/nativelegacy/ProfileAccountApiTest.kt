package space.nuvio.nativelegacy

import java.net.ServerSocket
import org.json.JSONArray
import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Test
import java.net.InetAddress

class ProfileAccountApiTest {
    private data class Call(val path: String, val body: ByteArray, val type: String)
    private fun fixture(respond: (String, ByteArray) -> String? = { _, _ -> null }, block: (String, MutableList<Call>, (String, Int) -> Unit) -> Unit) {
        val calls = java.util.Collections.synchronizedList(mutableListOf<Call>())
        val denied = java.util.concurrent.atomic.AtomicReference("")
        val denial = java.util.concurrent.atomic.AtomicInteger(403)
        val server = ServerSocket(0, 10, InetAddress.getByName("127.0.0.1"))
        val running = java.util.concurrent.atomic.AtomicBoolean(true)
        val worker = Thread {
            while (running.get()) try {
                server.accept().use { socket ->
                    socket.soTimeout = 5000
                    val input = socket.getInputStream()
                    val header = java.io.ByteArrayOutputStream()
                    while (!header.toString("US-ASCII").endsWith("\r\n\r\n")) {
                        val byte = input.read(); require(byte >= 0 && header.size() < 65536); header.write(byte)
                    }
                    val lines = header.toString("US-ASCII").split("\r\n")
                    val method = lines.first().split(" ")[0]
                    val path = lines.first().split(" ")[1]
                    val length = lines.firstOrNull { it.startsWith("Content-Length:", true) }?.substringAfter(":")?.trim()?.toInt() ?: 0
                    val body = ByteArray(length); var read = 0
                    while (read < length) { val n = input.read(body, read, length - read); require(n > 0); read += n }
                    val type = lines.firstOrNull { it.startsWith("Content-Type:", true) }?.substringAfter(":")?.trim().orEmpty()
                    calls.add(Call(if (method == "GET") "GET $path" else path, body, type))
                    val status = if (denied.get().isNotEmpty() && path.contains(denied.get())) denial.get() else 200
                    val custom = if (status == 200) respond(path, body) else null
                    val response = when {
                        status != 200 -> "{}"
                        custom != null -> custom
                        path.endsWith("get_avatar_catalog") -> """[{"id":"avatar_one","display_name":"One","storage_path":"one.png"},{"id":"bad","storage_path":"../private.png"}]"""
                        path.endsWith("sync_patch_profile") -> """[{"profile_index":2,"name":"Renamed","avatar_url":null}]"""
                        else -> "{}"
                    }.toByteArray()
                    socket.getOutputStream().apply {
                        write("HTTP/1.1 $status Result\r\nContent-Type: application/json\r\nContent-Length: ${response.size}\r\nConnection: close\r\n\r\n".toByteArray())
                        write(response); flush()
                    }
                }
            } catch (_: java.net.SocketException) { if (running.get()) throw IllegalStateException("Mock server socket failed") }
        }.apply { isDaemon = true; start() }
        try { block("http://127.0.0.1:${server.localPort}", calls) { path, code -> denied.set(path); denial.set(code) } }
        finally { running.set(false); server.close(); worker.join(1000) }
    }

    private fun config(base: String, readOnly: Boolean = false) = JSONObject()
        .put("base", base).put("token", "fixture-token").put("key", "fixture-key")
        .put("profile", 2).put("user", "fixture-user").put("readonly", readOnly)
        .put("photo", "https://example.com/current.png")

    @Test fun renameOnlyPatchesTheSelectedProfileAndPreservesPhoto() = fixture { base, calls, _ ->
        val saved = ProfileAccountApi(config(base)).save("Renamed", false, null, "https://example.com/current.png")
        assertEquals("https://example.com/current.png", saved.photo)
        assertEquals(1, calls.size)
        val body = JSONObject(String(calls.single().body))
        assertEquals(2, body.length()); assertEquals(2, body.getInt("p_profile_id")); assertEquals("Renamed", body.getString("p_name"))
        assertTrue(calls.single().path.endsWith("sync_patch_profile"))
    }
    @Test fun avatarSelectionUsesCatalogIdAndClearsCustomUrl() = fixture { base, calls, _ ->
        val api = ProfileAccountApi(config(base)); val avatar = api.avatars().single()
        assertEquals("avatar_one", avatar.id)
        api.save("Renamed", true, avatar, avatar.url)
        val body = JSONObject(String(calls.last().body))
        assertEquals("avatar_one", body.getString("p_avatar_id")); assertTrue(body.isNull("p_avatar_url"))
        assertTrue(body.getBoolean("p_avatar_id_provided")); assertTrue(body.getBoolean("p_avatar_url_provided"))
        assertFalse(body.has("p_profiles")); assertFalse(body.has("p_uses_primary_plugins"))
    }
    @Test fun personalPhotoUploadsOnlyToAnImmutableOwnedPath() = fixture { base, calls, _ ->
        val photo = byteArrayOf(1, 2, 3, 4); val api = ProfileAccountApi(config(base))
        val url = api.upload(photo)
        assertTrue(calls.single().path.startsWith("/storage/v1/object/avatars/fixture-user/profile-2-"))
        assertEquals("image/jpeg", calls.single().type); assertArrayEquals(photo, calls.single().body)
        api.save("Renamed", true, null, url)
        val body = JSONObject(String(calls.last().body))
        assertEquals(url, body.getString("p_avatar_url")); assertTrue(body.isNull("p_avatar_id"))
    }
    @Test fun uploadRefusalDoesNotPretendThatThePhotoSaved() = fixture { base, calls, deny ->
        deny("/storage/", 403)
        val error = runCatching { ProfileAccountApi(config(base)).upload(byteArrayOf(1)) }.exceptionOrNull()
        assertTrue(error?.message?.contains("does not allow personal photo uploads") == true)
        assertEquals(1, calls.size); assertFalse(calls.any { it.path.endsWith("sync_patch_profile") })
    }
    @Test fun readOnlyAndClosedEditorsNeverDispatchWrites() = fixture { base, calls, _ ->
        val api = ProfileAccountApi(config(base, true))
        assertTrue(runCatching { api.save("Renamed", false, null, "") }.isFailure)
        assertTrue(runCatching { api.upload(byteArrayOf(1)) }.isFailure)
        assertTrue(runCatching { ProfileAccountApi(config(base)) { false }.save("Renamed", false, null, "") }.isFailure)
        assertTrue(calls.isEmpty())
    }

    /** A tiny account: two profiles that sync_pull/push/delete keep in memory. */
    private class Account {
        val rows = mutableListOf(
            JSONObject("""{"profile_index":1,"name":"Main","avatar_color_hex":"#1E88E5","avatar_id":"avatar_one","avatar_url":null,"profile_background_id":"bg_member","profile_background_url":null,"uses_primary_addons":false,"uses_primary_plugins":false,"is_primary":true}"""),
            JSONObject("""{"profile_index":2,"name":"Kids","avatar_color_hex":"#43A047","avatar_id":null,"avatar_url":"https://example.com/current.png","profile_background_id":null,"profile_background_url":null,"uses_primary_addons":true,"uses_primary_plugins":true}"""))
        var deleteRemovesRow = false
        fun respond(path: String, body: ByteArray): String? = synchronized(this) { when {
            path.endsWith("sync_pull_profiles") -> JSONArray(rows.map { JSONObject(it.toString()) }).toString()
            path.endsWith("sync_push_profiles") -> {
                val sent = JSONObject(String(body)).getJSONArray("p_profiles")
                rows.clear(); for (i in 0 until sent.length()) rows.add(sent.getJSONObject(i)); "null"
            }
            path.endsWith("sync_delete_profile_data") -> {
                if (deleteRemovesRow) rows.removeAll { it.getInt("profile_index") == JSONObject(String(body)).getInt("p_profile_id") }; "null"
            }
            path.endsWith("get_member_profile_background_catalog") ->
                """[{"id":"bg_one","display_name":"Night","storage_path":"night.jpg","asset_version":1},{"id":"bad","storage_path":"../x"}]"""
            path.contains("/storage/v1/object/authenticated/") -> "image-bytes"
            path.endsWith("profile_pin") -> "null"
            else -> null
        } }
    }
    private fun accountFixture(account: Account, block: (String, MutableList<Call>, (String, Int) -> Unit) -> Unit) =
        fixture({ path, body -> account.respond(path, body) }, block)

    @Test fun colourChangePushesEveryProfileAndKeepsTheOthersUnchanged() {
        val account = Account()
        val before = JSONObject(account.rows[0].toString())
        accountFixture(account) { base, calls, _ ->
            val saved = ProfileAccountApi(config(base).put("client", "device-1")).save("Kids", false, null,
                "https://example.com/current.png", "#E53935", null)
            assertEquals(listOf("sync_pull_profiles", "sync_push_profiles", "sync_pull_profiles"), calls.map { it.path.substringAfterLast('/') })
            val push = JSONObject(String(calls[1].body))
            assertEquals(6, push.getInt("p_client_max_profiles")); assertEquals("device-1", push.getString("p_origin_client_id"))
            val profiles = push.getJSONArray("p_profiles"); assertEquals(2, profiles.length())
            val main = profiles.getJSONObject(0)
            for (key in listOf("name", "avatar_color_hex", "avatar_id", "profile_background_id", "uses_primary_addons", "uses_primary_plugins"))
                assertEquals(key, before.get(key).toString(), main.get(key).toString())
            assertTrue(main.isNull("avatar_url")); assertTrue(main.isNull("profile_background_url"))
            val kids = profiles.getJSONObject(1)
            assertEquals("#E53935", kids.getString("avatar_color_hex")); assertEquals("https://example.com/current.png", kids.getString("avatar_url"))
            assertTrue(kids.getBoolean("uses_primary_addons"))
            assertEquals("#E53935", saved.color); assertEquals("https://example.com/current.png", saved.photo); assertEquals("", saved.background)
        }
    }
    @Test fun backgroundLinkAndAvatarTravelInOnePush() {
        val account = Account()
        accountFixture(account) { base, calls, _ ->
            val api = ProfileAccountApi(config(base)); val avatar = api.avatars().single()
            val saved = api.save("Renamed", true, avatar, avatar.url, null,
                ProfileAccountApi.BackgroundChoice(null, "https://example.com/wall.jpg"))
            val kids = JSONObject(String(calls.first { it.path.endsWith("sync_push_profiles") }.body)).getJSONArray("p_profiles").getJSONObject(1)
            assertEquals("Renamed", kids.getString("name")); assertEquals("avatar_one", kids.getString("avatar_id"))
            assertTrue(kids.isNull("avatar_url")); assertEquals("https://example.com/wall.jpg", kids.getString("profile_background_url"))
            assertTrue(kids.isNull("profile_background_id"))
            assertEquals(avatar.url, saved.photo); assertEquals("https://example.com/wall.jpg", saved.background)
            assertEquals(1, calls.count { it.path.endsWith("sync_push_profiles") })
        }
    }
    @Test fun standardBackgroundClearsIdAndLink() {
        val account = Account()
        account.rows[1].put("profile_background_url", "https://example.com/old.jpg")
        accountFixture(account) { base, _, _ ->
            val saved = ProfileAccountApi(config(base)).save("Kids", false, null, "https://example.com/current.png", null,
                ProfileAccountApi.BackgroundChoice(null, null))
            assertTrue(account.rows[1].isNull("profile_background_url")); assertTrue(account.rows[1].isNull("profile_background_id"))
            assertEquals("", saved.background)
        }
    }
    @Test fun unexpectedProfileListsAreNeverPushed() {
        val account = Account()
        account.rows.add(JSONObject(account.rows[1].toString()))   // duplicate profile_index
        accountFixture(account) { base, calls, _ ->
            assertTrue(runCatching { ProfileAccountApi(config(base)).save("Kids", false, null, "", "#E53935") }.isFailure)
            assertFalse(calls.any { it.path.endsWith("sync_push_profiles") })
        }
    }
    @Test fun nameOnlySaveStillUsesTheSparsePatch() = fixture { base, calls, _ ->
        ProfileAccountApi(config(base)).save("Renamed", false, null, "https://example.com/current.png", null, null)
        assertEquals(listOf("sync_patch_profile"), calls.map { it.path.substringAfterLast('/') })
    }
    @Test fun avatarCatalogueCarriesItsColour() = fixture({ path, _ ->
        if (path.endsWith("get_avatar_catalog")) """[{"id":"a","display_name":"A","storage_path":"a.png","bg_color":"ff8800"},{"id":"b","storage_path":"b.png","bg_color":"nope"}]""" else null
    }) { base, _, _ ->
        val avatars = ProfileAccountApi(config(base)).avatars()
        assertEquals("#FF8800", avatars[0].color); assertNull(avatars[1].color)
    }
    @Test fun supporterBackgroundsAreReadWithTheSessionAndRejectUnsafePaths() {
        val account = Account()
        accountFixture(account) { base, calls, _ ->
            val api = ProfileAccountApi(config(base)); val list = api.backgrounds()
            assertEquals(listOf("bg_one"), list.map { it.id })
            assertEquals("image-bytes", String(api.backgroundImage(list.single())))
            assertEquals("GET /storage/v1/object/authenticated/membership-profile-backgrounds/night.jpg", calls.last().path)
            assertTrue(runCatching { api.backgroundImage(ProfileAccountApi.Background("x", "x", "../secret")) }.isFailure)
        }
    }
    @Test fun pinChangesSendTheCurrentPinOnlyWhenKnown() {
        val account = Account()
        accountFixture(account) { base, calls, _ ->
            val api = ProfileAccountApi(config(base))
            api.setPin("1234", null); api.setPin("5678", "1234"); api.clearPin("5678")
            val bodies = calls.map { JSONObject(String(it.body)) }
            assertEquals(listOf("set_profile_pin", "set_profile_pin", "clear_profile_pin"), calls.map { it.path.substringAfterLast('/') })
            assertFalse(bodies[0].has("p_current_pin")); assertEquals("1234", bodies[1].getString("p_current_pin"))
            assertEquals("5678", bodies[2].getString("p_current_pin")); assertTrue(bodies.all { it.getInt("p_profile_id") == 2 })
            assertTrue(runCatching { api.setPin("12a4", null) }.isFailure); assertEquals(3, calls.size)
        }
    }
    @Test fun wrongCurrentPinIsReportedPlainly() = fixture { base, _, deny ->
        deny("profile_pin", 400)
        val error = runCatching { ProfileAccountApi(config(base)).clearPin("0000") }.exceptionOrNull()
        assertEquals("The current PIN was not accepted. Try again.", error?.message)
    }
    @Test fun deleteRemovesDataThenPushesTheListWithoutTheProfile() {
        val account = Account()
        accountFixture(account) { base, calls, _ ->
            ProfileAccountApi(config(base).put("client", "device-1")).delete(primary = false, active = 1)
            assertEquals(listOf("sync_delete_profile_data", "sync_pull_profiles", "sync_push_profiles", "sync_pull_profiles"),
                calls.map { it.path.substringAfterLast('/') })
            assertEquals(listOf(1), account.rows.map { it.getInt("profile_index") })
            assertEquals("Main", account.rows[0].getString("name"))
        }
    }
    @Test fun deleteSkipsThePushWhenTheServerAlreadyRemovedTheRow() {
        val account = Account().apply { deleteRemovesRow = true }
        accountFixture(account) { base, calls, _ ->
            ProfileAccountApi(config(base)).delete(primary = false, active = 1)
            assertFalse(calls.any { it.path.endsWith("sync_push_profiles") })
        }
    }
    @Test fun mainActiveAndPrimaryProfilesAreNeverDeleted() = fixture { base, calls, _ ->
        assertTrue(runCatching { ProfileAccountApi(config(base)).delete(primary = false, active = 2) }.isFailure)
        assertTrue(runCatching { ProfileAccountApi(config(base)).delete(primary = true, active = 1) }.isFailure)
        assertTrue(runCatching { ProfileAccountApi(config(base).put("profile", 1)).delete(primary = false, active = 2) }.isFailure)
        assertTrue(calls.isEmpty())
    }
    @Test fun readOnlyBuildsNeverPushPinOrDelete() {
        val account = Account()
        accountFixture(account) { base, calls, _ ->
            val api = ProfileAccountApi(config(base, true))
            assertTrue(runCatching { api.save("Kids", false, null, "", "#E53935") }.isFailure)
            assertTrue(runCatching { api.setPin("1234", null) }.isFailure)
            assertTrue(runCatching { api.delete(false, 1) }.isFailure)
            assertTrue(calls.none { it.path.endsWith("push_profiles") || it.path.endsWith("profile_pin") || it.path.endsWith("delete_profile_data") })
        }
    }
}
