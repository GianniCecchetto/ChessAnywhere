def toggle_backlight(self):
    """
    Simule l'activation/désactivation du rétroéclairage.
    """
    # Logique pour le rétroéclairage
    # Note: Cette méthode est très spécifique. Si vous ajoutez plus de widgets,
    # il faudra améliorer la manière de référencer le bouton.
    backlight_btn = self.settings_menu.children.get("!ctkframe!ctkbutton")
    if backlight_btn:
        current_text = backlight_btn.cget("text")
        if "ON" in current_text:
            backlight_btn.configure(text="🔆 OFF")
        else:
            backlight_btn.configure(text="🔆 ON")