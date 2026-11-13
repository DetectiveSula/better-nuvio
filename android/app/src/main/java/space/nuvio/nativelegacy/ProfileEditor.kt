package space.nuvio.nativelegacy

import android.app.Activity
import android.app.Dialog
import android.content.Intent
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Color
import android.graphics.Matrix
import android.graphics.Typeface
import android.graphics.drawable.Drawable
import android.graphics.drawable.GradientDrawable
import android.graphics.drawable.StateListDrawable
import android.media.ExifInterface
import android.text.Editable
import android.text.TextWatcher
import android.view.Gravity
import android.view.KeyEvent
import android.view.View
import android.view.ViewGroup
import android.view.Window
import android.view.WindowManager
import android.view.inputmethod.EditorInfo
import android.widget.*
import org.json.JSONObject
import java.io.ByteArrayInputStream
import java.io.ByteArrayOutputStream
import java.net.HttpURLConnection
import java.net.URL
import java.util.concurrent.ConcurrentLinkedQueue

/**
 * Android/TV profile editor, opened by holding OK on a profile. It starts on the profile's
 * options (edit, PIN, delete, as official Nuvio TV does) and only an explicit Save, PIN
 * confirmation or Delete confirmation dispatches an account mutation.
 *
 * Every control is a focusable view with a visible focused state (accent fill plus outline),
 * so the remote's D-pad always shows where it is. Focus never leaves the panel.
 */
class ProfileEditor(private val activity: Activity) {
    private val events = ConcurrentLinkedQueue<String>()
    private val imageWorkers = java.util.concurrent.Executors.newFixedThreadPool(3) { r -> Thread(r, "profile-image").apply { isDaemon = true } }
    @Volatile private var session: Session? = null

    private enum class Tab { Avatar, Colour, Background }
    private sealed class Catalogue<out T> {
        object Loading : Catalogue<Nothing>()
        object Failed : Catalogue<Nothing>()
        data class Ready<T>(val items: List<T>) : Catalogue<T>()
    }

    private inner class Session(val config: JSONObject) {
        val api = ProfileAccountApi(config) { !closed }
        val request = config.getInt("request")
        val profile = config.getInt("profile")
        val readOnly = config.optBoolean("readonly")
        val primary = config.optBoolean("primary")
        val active = config.optInt("active", 0)
        val accent = ProfileEditPolicy.normalColor(config.optString("accent"))?.let { Color.parseColor(it) } ?: Color.rgb(45, 212, 191)
        var hasPin = config.optBoolean("hasPin")
        /** The PIN the person just typed to open this locked profile; memory only. */
        var currentPin: String? = config.optString("pin").takeIf { ProfileEditPolicy.validPin(it) }
        var dialog: Dialog? = null
        lateinit var panel: LinearLayout
        var view = "options"
        // Editor state.
        var originalName = config.optString("name")
        var name = originalName
        val originalColor = ProfileEditPolicy.normalColor(config.optString("color")) ?: "#1E88E5"
        var color = originalColor
        var avatars: Catalogue<ProfileAccountApi.Avatar> = Catalogue.Loading
        var backgrounds: Catalogue<ProfileAccountApi.Background> = Catalogue.Loading
        var avatar: ProfileAccountApi.Avatar? = null
        var photo = config.optString("photo")
        var changedPhoto = false
        var personal: ByteArray? = null
        var personalBitmap: Bitmap? = null
        val originalBackground = config.optString("background").takeIf { ProfileEditPolicy.validPhotoUrl(it) }.orEmpty()
        var background: ProfileAccountApi.BackgroundChoice? = null
        var tab = Tab.Avatar
        var focusedAvatar: String? = null
        var picking = false
        var busy = false
        var status = ""
        var statusError = false
        @Volatile var closed = false
        // Live views of the editor (rebuilt with it).
        var preview: ImageView? = null
        var previewInitial: TextView? = null
        var previewName: TextView? = null
        var statusView: TextView? = null
        var tabContent: FrameLayout? = null
        var tabButtons = mutableMapOf<Tab, TextView>()
        var avatarLabel: TextView? = null
        var pinEntry = StringBuilder()
        var pinFirst: String? = null
        var pinPurpose = "set"
    }

    @Synchronized fun open(payload: String): Boolean {
        if (session != null || activity.isFinishing || activity.isDestroyed) return false
        val current = try { Session(JSONObject(payload)) } catch (_: Exception) { return false }
        session = current
        activity.runOnUiThread { if (!current.closed) show(current) }
        return true
    }
    fun next(): String? = events.poll()
    @Synchronized fun close(): Boolean {
        val current = session ?: return true
        emit(JSONObject().put("request", current.request).put("saved", 0))
        current.closed = true; current.personal = null; current.currentPin = null; session = null
        activity.runOnUiThread { current.dialog?.dismiss() }
        return true
    }
    fun destroy() { close(); imageWorkers.shutdownNow() }
    private fun finish(current: Session, event: JSONObject? = null) {
        if (session !== current || current.closed) return
        emit(event ?: JSONObject().put("request", current.request).put("saved", 0))
        close()
    }
    private fun emit(value: JSONObject) {
        // Native JNI receives ASCII JSON; the C parser decodes surrogate pairs.
        events.add(buildString { value.toString().forEach { c ->
            if (c.code > 127) append("\\u%04x".format(java.util.Locale.ROOT, c.code)) else append(c)
        } })
    }

    // ---- styling --------------------------------------------------------------------------
    private fun dp(value: Int) = (value * activity.resources.displayMetrics.density).toInt()
    private fun dpf(value: Float) = value * activity.resources.displayMetrics.density
    private fun withAlpha(color: Int, alpha: Int) = Color.argb(alpha, Color.red(color), Color.green(color), Color.blue(color))
    private fun shape(fill: Int, radius: Int, stroke: Int = 0, strokeColor: Int = 0, oval: Boolean = false) = GradientDrawable().apply {
        if (oval) this.shape = GradientDrawable.OVAL else cornerRadius = dpf(radius.toFloat())
        setColor(fill); if (stroke > 0) setStroke(dp(stroke), strokeColor)
    }
    /** Resting: faint glass. Focused: accent-tinted fill plus a 2dp accent outline. */
    private fun focusSurface(current: Session, radius: Int, oval: Boolean = false, rest: Int = Color.argb(22, 255, 255, 255)): Drawable =
        StateListDrawable().apply {
            val on = shape(withAlpha(current.accent, 72), radius, 2, current.accent, oval)
            addState(intArrayOf(android.R.attr.state_focused), on)
            addState(intArrayOf(android.R.attr.state_pressed), on)
            addState(intArrayOf(), shape(rest, radius, 1, Color.argb(40, 255, 255, 255), oval))
        }
    private fun text(value: String, size: Float, color: Int = Color.WHITE, bold: Boolean = false) = TextView(activity).apply {
        text = value; textSize = size; setTextColor(color); if (bold) setTypeface(Typeface.DEFAULT, Typeface.BOLD)
    }
    private val secondary = Color.argb(178, 255, 255, 255)
    private val tertiary = Color.argb(128, 255, 255, 255)
    private fun button(current: Session, label: String, primary: Boolean = false, danger: Boolean = false, action: () -> Unit) =
        TextView(activity).apply {
            text = label; textSize = 16f; gravity = Gravity.CENTER; isFocusable = true; isClickable = true
            setTypeface(Typeface.DEFAULT, Typeface.BOLD)
            setTextColor(if (danger) Color.rgb(255, 150, 150) else Color.WHITE)
            background = focusSurface(current, 24, rest = if (primary) withAlpha(current.accent, 46) else Color.argb(22, 255, 255, 255))
            minHeight = dp(48); setPadding(dp(20), dp(10), dp(20), dp(10))
            setOnClickListener { if (!current.busy && !current.closed) action() }
        }
    private fun lp(w: Int = -1, h: Int = -2, top: Int = 0) = LinearLayout.LayoutParams(w, h).apply { topMargin = dp(top) }

    // ---- shell ----------------------------------------------------------------------------
    private fun show(current: Session) {
        val dialog = Dialog(activity, android.R.style.Theme_Black_NoTitleBar_Fullscreen)
        dialog.requestWindowFeature(Window.FEATURE_NO_TITLE)
        current.panel = LinearLayout(activity).apply {
            orientation = LinearLayout.VERTICAL; setPadding(dp(32), dp(28), dp(32), dp(28))
            background = shape(Color.argb(242, 22, 24, 28), 24, 1, Color.argb(48, 255, 255, 255))
        }
        val scroll = ScrollView(activity).apply { isFillViewport = true; addView(FrameLayout(activity).apply {
            setPadding(dp(16), dp(24), dp(16), dp(24))
            addView(current.panel, FrameLayout.LayoutParams(minOf(dp(960), activity.resources.displayMetrics.widthPixels - dp(32)), -2, Gravity.CENTER))
        }) }
        val root = FrameLayout(activity).apply { setBackgroundColor(Color.argb(217, 0, 0, 0)); addView(scroll) }
        dialog.setContentView(root)
        dialog.setOnKeyListener { _, code, event ->
            if (event.action != KeyEvent.ACTION_UP) return@setOnKeyListener code == KeyEvent.KEYCODE_BACK ||
                (current.view == "pin" && digit(code) != null)
            when {
                code == KeyEvent.KEYCODE_BACK -> { back(current); true }
                current.view == "pin" && digit(code) != null -> { pinKey(current, digit(code)!!); true }
                current.view == "pin" && code == KeyEvent.KEYCODE_DEL -> { pinKey(current, '<'); true }
                else -> false
            }
        }
        dialog.setOnDismissListener { if (!current.closed) finish(current) }
        current.dialog = dialog
        dialog.show()
        dialog.window?.apply {
            setBackgroundDrawableResource(android.R.color.transparent)
            setLayout(-1, -1)
            setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN or WindowManager.LayoutParams.SOFT_INPUT_ADJUST_PAN)
        }
        showOptions(current)
        Thread({
            val avatars = runCatching { current.api.avatars() }
            val backgrounds = runCatching { current.api.backgrounds() }
            activity.runOnUiThread {
                if (current.closed) return@runOnUiThread
                current.avatars = avatars.fold({ Catalogue.Ready(it) }, { Catalogue.Failed })
                // A non-member's catalogue call fails: that simply means "no supporter backgrounds".
                current.backgrounds = Catalogue.Ready(backgrounds.getOrDefault(emptyList()))
                current.avatar = (current.avatars as? Catalogue.Ready)?.items?.firstOrNull { it.url == current.photo }
                if (current.view == "editor") refreshTab(current)
            }
        }, "profile-catalogues").start()
    }
    private fun digit(code: Int): Char? = when (code) {
        in KeyEvent.KEYCODE_0..KeyEvent.KEYCODE_9 -> '0' + (code - KeyEvent.KEYCODE_0)
        in KeyEvent.KEYCODE_NUMPAD_0..KeyEvent.KEYCODE_NUMPAD_9 -> '0' + (code - KeyEvent.KEYCODE_NUMPAD_0)
        else -> null
    }
    private fun back(current: Session) {
        if (current.busy) return
        when (current.view) {
            "options" -> finish(current)
            "link" -> showEditor(current, focusTab = true)
            else -> showOptions(current)
        }
    }
    private fun setStatus(current: Session, value: String, error: Boolean = false) {
        current.status = value; current.statusError = error
        current.statusView?.apply { text = value; setTextColor(if (error) Color.rgb(255, 170, 170) else secondary)
            visibility = if (value.isEmpty()) View.GONE else View.VISIBLE }
    }
    private fun statusLine(current: Session) = text(current.status, 14f).also {
        current.statusView = it; setStatus(current, current.status, current.statusError)
    }
    private fun header(current: Session, label: String, title: String, trailing: View? = null) = LinearLayout(activity).apply {
        orientation = LinearLayout.HORIZONTAL; gravity = Gravity.CENTER_VERTICAL
        addView(LinearLayout(activity).apply {
            orientation = LinearLayout.VERTICAL
            addView(text(label, 15f, secondary, true))
            addView(text(title, 28f, Color.WHITE, true).apply { maxLines = 1; ellipsize = android.text.TextUtils.TruncateAt.END })
        }, LinearLayout.LayoutParams(0, -2, 1f))
        if (trailing != null) addView(trailing, LinearLayout.LayoutParams(-2, dp(52)))
    }
    private fun readOnlyNote(current: Session) = if (!current.readOnly) null else
        text("Read-only test build: account changes can't be saved.", 13f, tertiary)

    // ---- options ------------------------------------------------------------------------------
    private fun showOptions(current: Session) {
        current.view = "options"; current.pinEntry.clear(); current.pinFirst = null
        val panel = current.panel; panel.removeAllViews()
        current.panel.layoutParams = (current.panel.layoutParams as FrameLayout.LayoutParams).apply {
            width = minOf(dp(520), activity.resources.displayMetrics.widthPixels - dp(32)) }
        panel.addView(header(current, "Profile options", current.originalName))
        val first = button(current, "Edit profile") { showEditor(current) }
        panel.addView(first, lp(top = 24))
        panel.addView(button(current, if (current.hasPin) "Change PIN" else "Set PIN") {
            current.pinPurpose = "set"; showPin(current) }, lp(top = 10))
        if (current.hasPin) panel.addView(button(current, "Remove PIN") {
            if (current.currentPin != null) confirmRemovePin(current) else { current.pinPurpose = "clear"; showPin(current) }
        }, lp(top = 10))
        if (ProfileEditPolicy.canDelete(current.profile, current.primary, current.active))
            panel.addView(button(current, "Delete profile", danger = true) { confirmDelete(current) }, lp(top = 10))
        panel.addView(button(current, "Close") { finish(current) }, lp(top = 10))
        panel.addView(statusLine(current), lp(top = 14))
        readOnlyNote(current)?.let { panel.addView(it, lp(top = 8)) }
        if (!ProfileEditPolicy.canDelete(current.profile, current.primary, current.active))
            panel.addView(text(if (current.profile == current.active) "The profile in use can't be deleted. Switch profile first."
                else "The main profile can't be deleted.", 13f, tertiary), lp(top = 8))
        first.requestFocus()
    }

    // ---- editor --------------------------------------------------------------------------------
    private fun showEditor(current: Session, focusTab: Boolean = false) {
        current.view = "editor"; setStatus(current, "")
        val panel = current.panel; panel.removeAllViews()
        val screen = activity.resources.displayMetrics.widthPixels
        current.panel.layoutParams = (current.panel.layoutParams as FrameLayout.LayoutParams).apply { width = minOf(dp(960), screen - dp(32)) }
        val wide = screen - dp(32) >= dp(720)
        val save = button(current, "Save", primary = true) { save(current) }
        panel.addView(header(current, "Edit profile", current.originalName, save))
        val body = LinearLayout(activity).apply { orientation = if (wide) LinearLayout.HORIZONTAL else LinearLayout.VERTICAL }
        panel.addView(body, lp(top = 20))

        // Left: live preview, name, cancel.
        val left = LinearLayout(activity).apply { orientation = LinearLayout.VERTICAL; gravity = Gravity.CENTER_HORIZONTAL }
        val circle = FrameLayout(activity).apply { background = shape(Color.parseColor(current.color), 0, oval = true); clipToOutline = true }
        current.previewInitial = text("", 52f, Color.WHITE, true).apply { gravity = Gravity.CENTER }
        current.preview = ImageView(activity).apply { scaleType = ImageView.ScaleType.CENTER_CROP; contentDescription = "Profile photo" }
        circle.addView(current.previewInitial, FrameLayout.LayoutParams(-1, -1))
        circle.addView(current.preview, FrameLayout.LayoutParams(-1, -1))
        left.addView(circle, LinearLayout.LayoutParams(dp(132), dp(132)))
        current.previewName = text("", 22f, Color.WHITE, true).apply { gravity = Gravity.CENTER; maxLines = 2 }
        left.addView(current.previewName, lp(top = 14))
        val name = EditText(activity).apply {
            hint = "Profile name"; contentDescription = "Profile name"; setSingleLine(true); textSize = 17f
            setText(current.name); setTextColor(Color.WHITE); setHintTextColor(tertiary)
            imeOptions = EditorInfo.IME_ACTION_DONE; setSelectAllOnFocus(true)
            background = focusSurface(current, 14); setPadding(dp(16), dp(10), dp(16), dp(10))
            addTextChangedListener(object : TextWatcher {
                override fun beforeTextChanged(s: CharSequence?, a: Int, b: Int, c: Int) {}
                override fun onTextChanged(s: CharSequence?, a: Int, b: Int, c: Int) {}
                override fun afterTextChanged(s: Editable) {
                    val fitted = ProfileEditPolicy.clampName(s.toString())
                    if (fitted != s.toString()) { s.replace(0, s.length, fitted); setStatus(current, "Names can be up to 20 characters.") }
                    current.name = s.toString(); updatePreview(current)
                }
            })
        }
        left.addView(name, lp(top = 12, h = dp(52)))
        left.addView(text("Up to 20 characters", 12f, tertiary).apply { gravity = Gravity.CENTER }, lp(top = 6))
        left.addView(button(current, "Cancel") { showOptions(current) }, lp(top = 14))
        body.addView(left, if (wide) LinearLayout.LayoutParams(dp(260), -2) else lp())
        if (wide) body.addView(View(activity).apply { setBackgroundColor(Color.argb(40, 255, 255, 255)) },
            LinearLayout.LayoutParams(dp(1), -1).apply { leftMargin = dp(28); rightMargin = dp(28) })

        // Right: tabs.
        val right = LinearLayout(activity).apply { orientation = LinearLayout.VERTICAL }
        val tabs = LinearLayout(activity).apply { orientation = LinearLayout.HORIZONTAL; gravity = Gravity.CENTER }
        current.tabButtons.clear()
        for (t in Tab.values()) {
            val b = text(t.name, 16f, Color.WHITE, true).apply {
                gravity = Gravity.CENTER; isFocusable = true; isClickable = true; setPadding(dp(22), dp(8), dp(22), dp(8))
                setOnFocusChangeListener { _, has -> if (has && current.tab != t) { current.tab = t; refreshTab(current) } }
                setOnClickListener { current.tab = t; refreshTab(current) }
            }
            current.tabButtons[t] = b
            tabs.addView(b, LinearLayout.LayoutParams(-2, dp(44)).apply { leftMargin = dp(4); rightMargin = dp(4) })
        }
        right.addView(tabs)
        current.tabContent = FrameLayout(activity)
        right.addView(current.tabContent, lp(top = 14))
        body.addView(right, if (wide) LinearLayout.LayoutParams(0, -2, 1f) else lp(top = 20))
        panel.addView(statusLine(current), lp(top = 14))
        panel.addView(text("Changes sync with your Nuvio account.", 13f, tertiary), lp(top = 6))
        readOnlyNote(current)?.let { panel.addView(it, lp(top = 4)) }
        updatePreview(current); refreshTab(current)
        if (focusTab) current.tabButtons[current.tab]?.requestFocus() else name.requestFocus()
    }
    private fun updatePreview(current: Session) {
        val trimmed = current.name.trim()
        current.previewName?.apply { text = trimmed.ifEmpty { "Profile name" }; setTextColor(if (trimmed.isEmpty()) tertiary else Color.WHITE) }
        current.previewInitial?.text = trimmed.takeIf { it.isNotEmpty() }?.let { String(Character.toChars(it.codePointAt(0))).uppercase() } ?: "?"
        (current.preview?.parent as? View)?.background = shape(Color.parseColor(current.color), 0, oval = true)
        val view = current.preview ?: return
        val bitmap = current.personalBitmap
        if (bitmap != null) { view.tag = null; view.setImageBitmap(bitmap) }
        else if (view.tag != current.photo) loadImage(current, view, current.photo)
    }
    private fun refreshTab(current: Session) {
        val box = current.tabContent ?: return
        for ((t, b) in current.tabButtons) {
            b.background = focusSurface(current, 22, rest = if (t == current.tab) Color.argb(46, 255, 255, 255) else Color.TRANSPARENT)
            b.setTextColor(if (t == current.tab) Color.WHITE else secondary)
        }
        val keep = box.findFocus()?.let { f -> generateSequence(f) { it.parent as? View }.firstOrNull { it.tag is String && (it.tag as String).startsWith("k:") }?.tag }
        box.removeAllViews()
        val content = LinearLayout(activity).apply { orientation = LinearLayout.VERTICAL }
        when (current.tab) {
            Tab.Avatar -> avatarTab(current, content)
            Tab.Colour -> colourTab(current, content)
            Tab.Background -> backgroundTab(current, content)
        }
        box.addView(content)
        if (keep != null) content.findViewWithTag<View>(keep)?.requestFocus()
    }
    private fun columnsFor(current: Session, cell: Int): Int {
        val width = current.tabContent?.width?.takeIf { it > 0 } ?: (current.panel.layoutParams.width - dp(360))
        return maxOf(3, minOf(7, width / dp(cell)))
    }
    private fun tile(current: Session, size: Int, oval: Boolean, selected: Boolean, label: String, fill: View, click: () -> Unit) =
        FrameLayout(activity).apply {
            isFocusable = true; isClickable = true; contentDescription = label; tag = "k:$label"
            background = focusSurface(current, 16, oval, Color.TRANSPARENT); setPadding(dp(5), dp(5), dp(5), dp(5))
            addView(fill, FrameLayout.LayoutParams(-1, -1))
            if (selected) addView(text("✓", 14f, Color.BLACK, true).apply {
                gravity = Gravity.CENTER; background = shape(current.accent, 0, oval = true)
            }, FrameLayout.LayoutParams(dp(24), dp(24), Gravity.TOP or Gravity.END))
            setOnClickListener { if (!current.busy) click() }
            layoutParams = GridLayout.LayoutParams().apply { width = dp(size); height = dp(size); setMargins(dp(6), dp(6), dp(6), dp(6)) }
        }
    private fun note(value: String) = text(value, 13f, tertiary).apply { gravity = Gravity.CENTER }
    private fun grid(current: Session, cell: Int) = GridLayout(activity).apply { columnCount = columnsFor(current, cell + 12) }
    private fun scrollBox(child: View, maxHeight: Int) = object : ScrollView(activity) {
        override fun onMeasure(w: Int, h: Int) = super.onMeasure(w, View.MeasureSpec.makeMeasureSpec(maxHeight, View.MeasureSpec.AT_MOST))
    }.apply { addView(child); isVerticalScrollBarEnabled = true }

    private fun avatarTab(current: Session, content: LinearLayout) {
        content.addView(note("Choose an avatar"))
        when (val c = current.avatars) {
            is Catalogue.Loading -> content.addView(text("Loading avatars…", 16f, secondary).apply { gravity = Gravity.CENTER }, lp(h = dp(140)).apply { gravity = Gravity.CENTER })
            is Catalogue.Failed -> {
                content.addView(note("Avatars couldn't load."), lp(top = 24))
                content.addView(button(current, "Try again") {
                    current.avatars = Catalogue.Loading; refreshTab(current)
                    Thread({ val r = runCatching { current.api.avatars() }
                        activity.runOnUiThread { if (!current.closed) { current.avatars = r.fold({ Catalogue.Ready(it) }, { Catalogue.Failed }); refreshTab(current) } }
                    }, "profile-avatar-catalog").start()
                }, lp(-2, top = 10).apply { gravity = Gravity.CENTER_HORIZONTAL })
            }
            is Catalogue.Ready -> {
                val g = grid(current, 84)
                for (a in c.items) {
                    val image = ImageView(activity).apply { scaleType = ImageView.ScaleType.CENTER_CROP
                        background = shape(a.color?.let { Color.parseColor(it) } ?: Color.argb(30, 255, 255, 255), 0, oval = true); clipToOutline = true }
                    val t = tile(current, 84, true, current.avatar?.id == a.id && current.personalBitmap == null, a.name, image) {
                        if (current.avatar?.id == a.id) {   // OK again on the chosen avatar undoes it, as in official Nuvio.
                            current.avatar = null; current.photo = ""; current.color = current.originalColor
                        } else {
                            current.avatar = a; current.photo = a.url; a.color?.let { current.color = it }
                        }
                        current.personal = null; current.personalBitmap = null; current.changedPhoto = true
                        setStatus(current, ""); updatePreview(current); refreshTab(current)
                    }
                    t.setOnFocusChangeListener { _, has -> if (has) { current.focusedAvatar = a.name; current.avatarLabel?.apply { text = a.name; setTextColor(Color.WHITE) } } }
                    g.addView(t); loadImage(current, image, a.url)
                }
                if (c.items.isEmpty()) content.addView(note("No avatars on this account server."), lp(top = 16))
                else content.addView(scrollBox(g, dp(280)), lp(-2, top = 6).apply { gravity = Gravity.CENTER_HORIZONTAL })
                current.avatarLabel = text(current.focusedAvatar ?: "Move to an avatar to see its name", 18f,
                    if (current.focusedAvatar != null) Color.WHITE else tertiary, true).apply { gravity = Gravity.CENTER }
                content.addView(current.avatarLabel, lp(top = 8))
            }
        }
        val more = LinearLayout(activity).apply { orientation = LinearLayout.HORIZONTAL; gravity = Gravity.CENTER }
        fun extra(label: String, action: () -> Unit) = more.addView(button(current, label, action = action).apply { textSize = 14f; tag = "k:$label" },
            LinearLayout.LayoutParams(-2, dp(44)).apply { leftMargin = dp(4); rightMargin = dp(4) })
        extra("Personal photo") { choosePhoto(current) }
        extra("Photo link") { photoUrl(current) }
        extra("No photo") {
            current.avatar = null; current.personal = null; current.personalBitmap = null; current.photo = ""; current.changedPhoto = true
            updatePreview(current); refreshTab(current)
        }
        content.addView(more, lp(top = 12))
    }
    private fun colourTab(current: Session, content: LinearLayout) {
        content.addView(note("Choose the profile colour"))
        val g = grid(current, 72)
        for (hex in ProfileEditPolicy.COLORS) {
            val swatch = View(activity).apply { background = shape(Color.parseColor(hex), 0, oval = true) }
            g.addView(tile(current, 72, true, current.color.equals(hex, true), hex, swatch) {
                current.color = hex; updatePreview(current); refreshTab(current)
            })
        }
        content.addView(g, lp(-2, top = 8).apply { gravity = Gravity.CENTER_HORIZONTAL })
        content.addView(note("Choosing an avatar also picks its colour."), lp(top = 10))
    }
    private fun backgroundTab(current: Session, content: LinearLayout) {
        content.addView(note("Choose the background behind this profile"))
        val chosen = current.background
        val currentUrl = chosen?.url ?: if (chosen == null) current.originalBackground else ""
        val currentId = chosen?.id
        val g = grid(current, 120)
        fun card(label: String, selected: Boolean, fill: View, click: () -> Unit) {
            val box = FrameLayout(activity).apply { addView(fill, FrameLayout.LayoutParams(-1, -1))
                addView(text(label, 12f, Color.WHITE, true).apply { gravity = Gravity.CENTER; setShadowLayer(4f, 0f, 1f, Color.BLACK)
                    setBackgroundColor(Color.argb(110, 0, 0, 0)) }, FrameLayout.LayoutParams(-1, -2, Gravity.BOTTOM)) }
            g.addView(tile(current, 120, false, selected, label, box, click))
        }
        card("Standard", currentUrl.isEmpty() && currentId == null,
            View(activity).apply { background = shape(withAlpha(Color.parseColor(current.color), 140), 12) }) {
            current.background = ProfileAccountApi.BackgroundChoice(null, null); refreshTab(current)
        }
        if (current.originalBackground.isNotEmpty()) {
            val image = ImageView(activity).apply { scaleType = ImageView.ScaleType.CENTER_CROP; clipToOutline = true; background = shape(Color.DKGRAY, 12) }
            card("Current link", currentUrl == current.originalBackground && currentId == null, image) {
                current.background = null; refreshTab(current)
            }
            loadImage(current, image, current.originalBackground, square = false)
        }
        (current.backgrounds as? Catalogue.Ready)?.items?.forEach { b ->
            val image = ImageView(activity).apply { scaleType = ImageView.ScaleType.CENTER_CROP; clipToOutline = true; background = shape(Color.DKGRAY, 12) }
            card(b.name, currentId == b.id, image) {
                current.background = ProfileAccountApi.BackgroundChoice(b.id, null); refreshTab(current)
            }
            imageWorkers.execute {
                val bitmap = runCatching { decodeScaled(current.api.backgroundImage(b), 360) }.getOrNull()
                activity.runOnUiThread { if (!current.closed && bitmap != null) image.setImageBitmap(bitmap) }
            }
        }
        card(if (currentUrl.isNotEmpty() && currentUrl != current.originalBackground) "New link" else "Image link…",
            currentUrl.isNotEmpty() && currentUrl != current.originalBackground,
            text("🔗", 28f).apply { gravity = Gravity.CENTER; background = shape(Color.argb(30, 255, 255, 255), 12) }) {
            linkDialog(current, "Background image link", currentUrl) { url ->
                current.background = ProfileAccountApi.BackgroundChoice(null, url); refreshTab(current)
            }
        }
        content.addView(scrollBox(g, dp(300)), lp(-2, top = 8).apply { gravity = Gravity.CENTER_HORIZONTAL })
        if (current.backgrounds == Catalogue.Loading) content.addView(note("Loading supporter backgrounds…"), lp(top = 6))
        if ((current.backgrounds as? Catalogue.Ready)?.items?.isNotEmpty() == true)
            content.addView(note("Supporter backgrounds appear in official Nuvio apps. Nuvio+ shows image-link backgrounds."), lp(top = 8))
    }

    private fun linkDialog(current: Session, title: String, initial: String, done: (String) -> Unit) {
        current.view = "link"
        val panel = current.panel; panel.removeAllViews()
        panel.addView(header(current, title, "Paste or type an https:// link"))
        val input = EditText(activity).apply { setSingleLine(); hint = "https://…"; setText(initial); textSize = 17f
            setTextColor(Color.WHITE); setHintTextColor(tertiary); background = focusSurface(current, 14); setPadding(dp(16), dp(10), dp(16), dp(10))
            imeOptions = EditorInfo.IME_ACTION_DONE }
        panel.addView(input, lp(h = dp(54), top = 20))
        val row = LinearLayout(activity).apply { orientation = LinearLayout.HORIZONTAL; gravity = Gravity.END }
        row.addView(button(current, "Cancel") { showEditor(current, focusTab = true) }, LinearLayout.LayoutParams(-2, dp(48)))
        row.addView(button(current, "Use link", primary = true) {
            val url = input.text.toString().trim()
            if (url.isEmpty() || !ProfileEditPolicy.validPhotoUrl(url)) setStatus(current, "Enter a valid http:// or https:// image link.", true)
            else { showEditor(current, focusTab = true); done(url) }
        }, LinearLayout.LayoutParams(-2, dp(48)).apply { leftMargin = dp(10) })
        panel.addView(row, lp(top = 16))
        panel.addView(statusLine(current), lp(top = 10))
        setStatus(current, "")
        input.requestFocus()
    }
    private fun photoUrl(current: Session) = linkDialog(current, "Photo link", current.photo) { url ->
        current.photo = url; current.personal = null; current.personalBitmap = null; current.avatar = null; current.changedPhoto = true
        updatePreview(current)
    }

    @Suppress("DEPRECATION") private fun choosePhoto(current: Session) {
        try {
            current.picking = true
            activity.startActivityForResult(Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
                type = "image/*"; addCategory(Intent.CATEGORY_OPENABLE)
            }, PHOTO_REQUEST)
        } catch (_: Exception) { current.picking = false; setStatus(current, "No photo picker is available on this device. Use a photo link instead.", true) }
    }
    fun result(code: Int, result: Int, data: Intent?): Boolean {
        if (code != PHOTO_REQUEST) return false
        val current = session ?: return true
        if (!current.picking || current.closed) return true
        current.picking = false
        val uri = data?.data
        if (result != Activity.RESULT_OK || uri == null) return true
        Thread({
            val converted = runCatching {
                val bytes = activity.contentResolver.openInputStream(uri)?.use { input ->
                    val output = ByteArrayOutputStream(); val buffer = ByteArray(8192)
                    while (true) { val n = input.read(buffer); if (n < 0) break
                        require(output.size() + n <= 20 * 1024 * 1024); output.write(buffer, 0, n) }
                    output.toByteArray()
                } ?: error("Unable to open photo")
                preparePhoto(bytes)
            }
            activity.runOnUiThread {
                if (!current.closed) converted.onSuccess { bytes ->
                    current.personal = bytes; current.avatar = null; current.changedPhoto = true
                    current.personalBitmap = BitmapFactory.decodeByteArray(bytes, 0, bytes.size); setStatus(current, "")
                    updatePreview(current); if (current.view == "editor") refreshTab(current)
                }.onFailure { setStatus(current, "Could not read this photo. Choose a JPEG, PNG or WebP image under 20 MB.", true) }
            }
        }, "profile-photo-read").start()
        return true
    }

    private fun lock(current: Session, busy: Boolean) {
        current.busy = busy; current.dialog?.setCancelable(!busy)
    }
    private fun save(current: Session) {
        if (current.busy) return
        val name = current.name.trim()
        if (!ProfileEditPolicy.validName(name)) { setStatus(current, "Enter a name (1–20 characters).", true); return }
        val color = current.color.takeIf { !it.equals(current.originalColor, true) }
        val background = current.background?.takeIf { it.id != null || (it.url ?: "") != current.originalBackground }
        if (name == current.originalName && !current.changedPhoto && color == null && background == null) { finish(current); return }
        lock(current, true); setStatus(current, "Saving…")
        Thread({
            val result = runCatching {
                current.personal?.let { current.photo = current.api.upload(it); current.personal = null }
                current.api.save(name, current.changedPhoto, current.avatar, current.photo, color, background)
            }
            activity.runOnUiThread {
                if (current.closed) return@runOnUiThread
                lock(current, false)
                result.onSuccess { saved ->
                    val event = JSONObject().put("request", current.request).put("saved", 1)
                        .put("name", saved.name).put("photo", saved.photo)
                    saved.color?.let { event.put("color", it) }
                    saved.background?.let { event.put("background", it) }
                    finish(current, event)
                }.onFailure { setStatus(current, it.message ?: "Could not save your changes. Try again.", true) }
            }
        }, "profile-account-save").start()
    }

    // ---- PIN -------------------------------------------------------------------------------------
    private fun showPin(current: Session) {
        current.view = "pin"; current.pinEntry.clear()
        val panel = current.panel; panel.removeAllViews()
        current.panel.layoutParams = (current.panel.layoutParams as FrameLayout.LayoutParams).apply {
            width = minOf(dp(520), activity.resources.displayMetrics.widthPixels - dp(32)) }
        val title = when {
            current.pinPurpose == "clear" -> "Enter the current PIN"
            current.pinFirst != null -> "Enter the new PIN again"
            else -> "Enter a new 4-digit PIN"
        }
        panel.addView(header(current, when { current.pinPurpose == "clear" -> "Remove PIN"; current.hasPin -> "Change PIN"; else -> "Set PIN" }, current.originalName))
        panel.addView(text(title, 18f, secondary).apply { gravity = Gravity.CENTER }, lp(top = 18))
        val dots = LinearLayout(activity).apply { orientation = LinearLayout.HORIZONTAL; gravity = Gravity.CENTER; tag = "dots" }
        repeat(ProfileEditPolicy.PIN_LENGTH) { dots.addView(View(activity), LinearLayout.LayoutParams(dp(18), dp(18)).apply { leftMargin = dp(10); rightMargin = dp(10) }) }
        panel.addView(dots, lp(top = 16))
        val pad = GridLayout(activity).apply { columnCount = 3 }
        var five: View? = null
        for (k in listOf('1', '2', '3', '4', '5', '6', '7', '8', '9', 'C', '0', '<')) {
            val label = when (k) { 'C' -> "Clear"; '<' -> "⌫"; else -> k.toString() }
            val b = button(current, label) { pinKey(current, k) }.apply { textSize = if (k.isDigit()) 22f else 16f
                layoutParams = GridLayout.LayoutParams().apply { width = dp(96); height = dp(60); setMargins(dp(6), dp(6), dp(6), dp(6)) } }
            if (k == '5') five = b
            pad.addView(b)
        }
        panel.addView(pad, lp(-2, top = 16).apply { gravity = Gravity.CENTER_HORIZONTAL })
        panel.addView(button(current, "Cancel") { showOptions(current) }, lp(top = 14))
        panel.addView(statusLine(current), lp(top = 10))
        readOnlyNote(current)?.let { panel.addView(it, lp(top = 6)) }
        drawDots(current); five?.requestFocus()
    }
    private fun drawDots(current: Session) {
        val dots = current.panel.findViewWithTag<LinearLayout>("dots") ?: return
        for (i in 0 until dots.childCount) dots.getChildAt(i).background =
            shape(if (i < current.pinEntry.length) current.accent else Color.argb(50, 255, 255, 255), 0, oval = true)
    }
    private fun pinKey(current: Session, key: Char) {
        if (current.busy) return
        when (key) {
            'C' -> current.pinEntry.clear()
            '<' -> if (current.pinEntry.isNotEmpty()) current.pinEntry.setLength(current.pinEntry.length - 1)
            else -> if (current.pinEntry.length < ProfileEditPolicy.PIN_LENGTH) current.pinEntry.append(key)
        }
        drawDots(current)
        if (current.pinEntry.length < ProfileEditPolicy.PIN_LENGTH) return
        val pin = current.pinEntry.toString()
        when {
            current.pinPurpose == "clear" -> { current.currentPin = pin; confirmRemovePin(current) }
            current.pinFirst == null -> { current.pinFirst = pin; showPin(current) }
            current.pinFirst != pin -> { current.pinFirst = null; showPin(current); setStatus(current, "The PINs didn't match. Try again.", true) }
            else -> {
                val previous = current.currentPin
                lock(current, true); setStatus(current, "Saving PIN…")
                Thread({
                    val r = runCatching { current.api.setPin(pin, previous) }
                    activity.runOnUiThread {
                        if (current.closed) return@runOnUiThread
                        lock(current, false); current.pinFirst = null
                        r.onSuccess {
                            current.hasPin = true; current.currentPin = pin
                            emit(JSONObject().put("request", current.request).put("final", 0).put("pin", 1))
                            showOptions(current); setStatus(current, "PIN saved for ${current.originalName}.")
                        }.onFailure { showPin(current); setStatus(current, it.message ?: "Could not save the PIN. Try again.", true) }
                    }
                }, "profile-pin").start()
            }
        }
    }
    private fun confirm(current: Session, title: String, message: String, action: String, run: () -> Unit) {
        current.view = "confirm"
        val panel = current.panel; panel.removeAllViews()
        panel.addView(header(current, title, current.originalName))
        panel.addView(text(message, 16f, secondary), lp(top = 16))
        val row = LinearLayout(activity).apply { orientation = LinearLayout.HORIZONTAL; gravity = Gravity.END }
        val cancel = button(current, "Cancel") { showOptions(current) }
        row.addView(cancel, LinearLayout.LayoutParams(-2, dp(48)))
        row.addView(button(current, action, danger = true) { run() }, LinearLayout.LayoutParams(-2, dp(48)).apply { leftMargin = dp(10) })
        panel.addView(row, lp(top = 20))
        panel.addView(statusLine(current), lp(top = 10)); setStatus(current, "")
        readOnlyNote(current)?.let { panel.addView(it, lp(top = 6)) }
        cancel.requestFocus()   // The safe choice is where focus lands.
    }
    private fun confirmRemovePin(current: Session) = confirm(current, "Remove PIN",
        "Anyone using this TV will be able to open this profile without a PIN.", "Remove PIN") {
        val pin = current.currentPin
        lock(current, true); setStatus(current, "Removing PIN…")
        Thread({
            val r = runCatching { current.api.clearPin(pin) }
            activity.runOnUiThread {
                if (current.closed) return@runOnUiThread
                lock(current, false)
                r.onSuccess {
                    current.hasPin = false; current.currentPin = null
                    emit(JSONObject().put("request", current.request).put("final", 0).put("pin", 0))
                    showOptions(current); setStatus(current, "PIN removed from ${current.originalName}.")
                }.onFailure { current.currentPin = null; setStatus(current, it.message ?: "Could not remove the PIN. Try again.", true) }
            }
        }, "profile-pin").start()
    }
    private fun confirmDelete(current: Session) = confirm(current, "Delete profile",
        "Delete ${current.originalName}? Its watch progress, library and settings are removed from your Nuvio account. This can't be undone.",
        "Delete") {
        lock(current, true); setStatus(current, "Deleting…")
        Thread({
            val r = runCatching { current.api.delete(current.primary, current.active) }
            activity.runOnUiThread {
                if (current.closed) return@runOnUiThread
                lock(current, false)
                r.onSuccess { finish(current, JSONObject().put("request", current.request).put("deleted", 1)) }
                    .onFailure { setStatus(current, it.message ?: "Could not delete this profile. Try again.", true) }
            }
        }, "profile-delete").start()
    }

    // ---- images ----------------------------------------------------------------------------------
    private fun loadImage(current: Session, view: ImageView, url: String, square: Boolean = true) {
        view.tag = url
        if (url.isEmpty()) { view.setImageDrawable(null); return }
        if (!ProfileEditPolicy.validPhotoUrl(url)) return
        imageWorkers.execute {
            if (current.closed) return@execute
            val bitmap = runCatching {
                val connection = URL(url).openConnection() as HttpURLConnection
                connection.connectTimeout = 10_000; connection.readTimeout = 10_000; connection.instanceFollowRedirects = false
                try {
                    require(connection.responseCode in 200..299)
                    val bytes = connection.inputStream.use { input ->
                        val out = ByteArrayOutputStream(); val b = ByteArray(8192)
                        while (true) { val n = input.read(b); if (n < 0) break; require(out.size() + n <= 4 * 1024 * 1024); out.write(b, 0, n) }; out.toByteArray()
                    }
                    if (square) { val prepared = preparePhoto(bytes); BitmapFactory.decodeByteArray(prepared, 0, prepared.size) }
                    else decodeScaled(bytes, 360)
                } finally { connection.disconnect() }
            }.getOrNull()
            activity.runOnUiThread { if (!current.closed && view.tag == url && bitmap != null) view.setImageBitmap(bitmap) }
        }
    }
    companion object {
        private const val PHOTO_REQUEST = 7354
        private fun decodeScaled(bytes: ByteArray, maxSide: Int): Bitmap? {
            val bounds = BitmapFactory.Options().apply { inJustDecodeBounds = true }
            BitmapFactory.decodeByteArray(bytes, 0, bytes.size, bounds)
            if (bounds.outWidth <= 0 || bounds.outHeight <= 0 || bounds.outWidth.toLong() * bounds.outHeight > 100_000_000L) return null
            var sample = 1
            while (bounds.outWidth / (sample * 2) >= maxSide && bounds.outHeight / (sample * 2) >= maxSide) sample *= 2
            return BitmapFactory.decodeByteArray(bytes, 0, bytes.size, BitmapFactory.Options().apply { inSampleSize = sample })
        }
        private fun preparePhoto(bytes: ByteArray): ByteArray {
            val bounds = BitmapFactory.Options().apply { inJustDecodeBounds = true }
            BitmapFactory.decodeByteArray(bytes, 0, bytes.size, bounds)
            require(bounds.outWidth > 0 && bounds.outHeight > 0 && bounds.outWidth.toLong() * bounds.outHeight <= 100_000_000L)
            var sample = 1
            while (bounds.outWidth / sample > 1024 || bounds.outHeight / sample > 1024) sample *= 2
            val decoded = BitmapFactory.decodeByteArray(bytes, 0, bytes.size, BitmapFactory.Options().apply { inSampleSize = sample }) ?: error("Invalid photo")
            val orientation = runCatching { ExifInterface(ByteArrayInputStream(bytes)).getAttributeInt(ExifInterface.TAG_ORIENTATION, 1) }.getOrDefault(1)
            val matrix = Matrix().apply { when (orientation) {
                2 -> setScale(-1f, 1f); 3 -> setRotate(180f); 4 -> setScale(1f, -1f)
                5 -> { setRotate(90f); postScale(-1f, 1f) }; 6 -> setRotate(90f)
                7 -> { setRotate(270f); postScale(-1f, 1f) }; 8 -> setRotate(270f)
            } }
            val rotated = Bitmap.createBitmap(decoded, 0, 0, decoded.width, decoded.height, matrix, true)
            val side = minOf(rotated.width, rotated.height)
            val square = Bitmap.createBitmap(rotated, (rotated.width - side) / 2, (rotated.height - side) / 2, side, side)
            val resized = Bitmap.createScaledBitmap(square, minOf(512, side), minOf(512, side), true)
            val output = ByteArrayOutputStream(); require(resized.compress(Bitmap.CompressFormat.JPEG, 88, output))
            setOf(decoded, rotated, square, resized).forEach { it.recycle() }
            return output.toByteArray()
        }
    }
}
