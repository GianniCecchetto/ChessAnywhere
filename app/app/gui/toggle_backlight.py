def toggle_backlight(self):
    """
    activation/désactivation du rétroéclairage.
    """
    backlight_btn = self.settings_menu.children.get("!ctkframe!ctkbutton")
    if backlight_btn:
        current_text = backlight_btn.cget("text")
        if "ON" in current_text:
            backlight_btn.configure(text="🔆 OFF")
        else:
            backlight_btn.configure(text="🔆 ON")