import customtkinter as ctk
import tkinter as tk
from . import app_color as c 
from .toggle_backlight import toggle_backlight

def create_settings_menu(app):
    """
    Crée le menu des paramètres qui glisse depuis la droite.
    """
    app.settings_menu = ctk.CTkFrame(app, fg_color=c.SETTINGS_BTN, corner_radius=0)
    # Positionne le menu en dehors de l'écran, pour qu'il ne soit pas visible au début.
    app.settings_menu.grid(row=1, column=2, sticky="nsew")
    app.settings_menu.grid_remove() # Cache le widget
    app.settings_menu.pack_propagate(False)
    app.settings_menu.grid_columnconfigure(0, weight=1)
    app.settings_menu.grid_rowconfigure(0, weight=1)
        
    # Conteneur pour le contenu du menu
    menu_content_frame = ctk.CTkFrame(app.settings_menu, fg_color="transparent")
    menu_content_frame.grid(row=0, column=0, sticky="nsew", padx=20, pady=20)
        
    # Bouton Paramètres (en haut, grande icône)
    settings_icon_label = ctk.CTkLabel(menu_content_frame, text="⚙️", font=("Arial", 50, "bold"), text_color="white")
    settings_icon_label.pack(pady=(0, 20))

    # Thème de l'application
    app_theme_frame = ctk.CTkFrame(menu_content_frame, fg_color="transparent")
    app_theme_frame.pack(fill="x", pady=10)
    app_theme_frame.columnconfigure(0, weight=1)
    app_theme_frame.columnconfigure(1, weight=1)

    app_theme_option = ctk.CTkOptionMenu(app_theme_frame, values=["Blue", "Dark", "Light"], 
                                             fg_color=c.LEFT_PANEL_BG, text_color="black")
    app_theme_option.grid(row=0, column=0, sticky="ew", padx=(0, 5))
    app_theme_label = ctk.CTkLabel(app_theme_frame, text="App Theme", text_color="white")
    app_theme_label.grid(row=0, column=1, sticky="w", padx=(5, 0))

    # Thème de l'échiquier
    board_theme_frame = ctk.CTkFrame(menu_content_frame, fg_color="transparent")
    board_theme_frame.pack(fill="x", pady=10)
    board_theme_frame.columnconfigure(0, weight=1)
    board_theme_frame.columnconfigure(1, weight=1)
        
    board_theme_option = ctk.CTkOptionMenu(board_theme_frame, values=["Blue", "Gray", "Green"],
                                               fg_color=c.LEFT_PANEL_BG, text_color="black")
    board_theme_option.grid(row=0, column=0, sticky="ew", padx=(0, 5))
    board_theme_label = ctk.CTkLabel(board_theme_frame, text="Board Theme", text_color="white")
    board_theme_label.grid(row=0, column=1, sticky="w", padx=(5, 0))

    # Bouton Rétroéclairage
    backlight_frame = ctk.CTkFrame(menu_content_frame, fg_color="transparent")
    backlight_frame.pack(fill="x", pady=10)
    backlight_frame.columnconfigure(0, weight=1)
    backlight_frame.columnconfigure(1, weight=1)

    backlight_btn = ctk.CTkButton(backlight_frame, text="🔆 ON", fg_color=c.LEFT_PANEL_BG,
                                  text_color="black", hover_color=c.DARK_BTN_HOVER,
                                  command=toggle_backlight)
    backlight_btn.grid(row=0, column=0, sticky="ew", padx=(0, 5))
    backlight_label = ctk.CTkLabel(backlight_frame, text="Backlight", text_color="white")
    backlight_label.grid(row=0, column=1, sticky="w", padx=(5, 0))

def toggle_settings_menu(app):
    """
    Lance l'animation pour ouvrir ou fermer le menu des paramètres.
    """
    if app.settings_menu_is_open:
        app.settings_menu_is_open = False
        animate_slide_out(app)
    else:
        app.settings_menu.grid()  # Affiche le widget avant l'animation
        app.settings_menu_is_open = True
        animate_slide_in(app)

def animate_slide_in(app, width=0):
    """
    Anime le menu pour qu'il glisse vers l'intérieur.
    
    if width < app.settings_menu_width:
        width += 20
        if width > app.settings_menu_width:
            width = app.settings_menu_width
        app.after(10, lambda: animate_slide_in(app,width))"""
    app.grid_columnconfigure(2, minsize=width)

def animate_slide_out(app, width=None):
    """
    Anime le menu pour qu'il glisse vers l'extérieur.
   
    if width is None:
        width = app.settings_menu_width
        
    if width > 0:
        width -= 20
        if width < 0:
            width = 0
        app.grid_columnconfigure(2, minsize=width)
        app.after(10, lambda: animate_slide_out(app,width))
    else: """
    app.settings_menu.grid_remove() # Cache le widget à la fin de l'animation
