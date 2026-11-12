package space.nuvio.nativelegacy

import android.app.Activity
import android.app.Dialog
import android.graphics.Color
import android.graphics.Typeface
import android.graphics.drawable.GradientDrawable
import android.view.Gravity
import android.view.ViewGroup
import android.view.Window
import android.widget.Button
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView

/** Uses no account or provider state. The only write acknowledges this device's setup. */
object PhoneLayoutSetup {
    fun showIfNeeded(activity: Activity, detectedMobile: Boolean, mobileInterface: Boolean) {
        val preferences = activity.getSharedPreferences("phone-layout-setup", Activity.MODE_PRIVATE)
        if (!PhoneLayoutSetupPolicy.shouldShow(detectedMobile, mobileInterface,
                preferences.getBoolean("completed-v1", false))) return
        val density = activity.resources.displayMetrics.density
        fun dp(value: Int) = (value * density + .5f).toInt()
        fun surface(colour: String, radius: Int, stroke: Boolean = false) = GradientDrawable().apply {
            setColor(Color.parseColor(colour)); cornerRadius = dp(radius).toFloat()
            if (stroke) setStroke(dp(1), Color.parseColor("#646468"))
        }
        val dialog = Dialog(activity)
        dialog.requestWindowFeature(Window.FEATURE_NO_TITLE)
        dialog.setCancelable(false)
        val content = LinearLayout(activity).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(dp(28), dp(36), dp(28), dp(32))
            gravity = Gravity.CENTER_VERTICAL
        }
        fun text(value: String, size: Float, colour: String, top: Int, bold: Boolean = false) =
            TextView(activity).apply {
                text = value; textSize = size; setTextColor(Color.parseColor(colour))
                if (bold) setTypeface(typeface, Typeface.BOLD)
                content.addView(this, LinearLayout.LayoutParams(-1, -2).apply { topMargin = dp(top) })
            }
        text("NUVIO", 20f, "#F5F5F7", 0, true).letterSpacing = .18f
        text("PHONE DETECTED", 11f, "#B3D3BF", 64).letterSpacing = .12f
        text("Made for your phone.", 32f, "#F5F5F7", 18, true)
        text("The same Nuvio interface, adapted for touch and a smaller screen.",
            16f, "#B7B7BF", 18)
        TextView(activity).apply {
            text = "Phone layout selected\n\nTouch controls · Portrait & landscape"
            textSize = 15f; setTextColor(Color.parseColor("#F5F5F7"))
            setPadding(dp(20), dp(20), dp(20), dp(20))
            background = surface("#242426", 18, true)
            content.addView(this, LinearLayout.LayoutParams(-1, -2).apply { topMargin = dp(30) })
        }
        val continueButton = Button(activity).apply {
            text = "Continue"; isAllCaps = false; textSize = 16f
            setTextColor(Color.parseColor("#171719"))
            background = surface("#F5F5F7", 28)
            minimumHeight = dp(52)
            setOnClickListener {
                preferences.edit().putBoolean("completed-v1", true).apply()
                dialog.dismiss()
            }
        }
        content.addView(continueButton, LinearLayout.LayoutParams(-1, dp(54)).apply { topMargin = dp(28) })
        text("You can change your layout in Settings.", 12f, "#9898A3", 16).gravity = Gravity.CENTER
        val scroll = ScrollView(activity).apply {
            isFillViewport = true
            setBackgroundColor(Color.parseColor("#0D0D0F"))
            addView(content, ViewGroup.LayoutParams(-1, -2))
        }
        dialog.setContentView(scroll)
        if (activity.isFinishing || activity.isDestroyed) return
        dialog.show()
        dialog.window?.apply {
            setBackgroundDrawableResource(android.R.color.transparent)
            val width = minOf(activity.resources.displayMetrics.widthPixels - dp(24), dp(420))
            setLayout(width.coerceAtLeast(dp(240)), ViewGroup.LayoutParams.MATCH_PARENT)
        }
        continueButton.requestFocus()
    }
}
