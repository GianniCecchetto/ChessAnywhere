import customtkinter as ctk
import tkinter as tk
from . import app_color as c
from ..uart.uart_com  import send_command
import os
import sys

from lib.uart_fmt.python_doc import board_com_ctypes as cb

def set_brightness(value):
    """
    Envoie la valeur de luminosité à l'UART.
    La valeur est convertie en un entier.
    """
    brightness_value = int(value)
    command = cb.fmt_led_bright(brightness_value)
    send_command(command)
    print(f"Commande de luminosité envoyée : {command}")

def update_game_list_button_colors(app, selected_colors):
    """
    Met à jour les couleurs des boutons dans la liste des jeux.
    """
    if hasattr(app, 'games_list_frame'):
        for widget in app.games_list_frame.winfo_children():
            if isinstance(widget, ctk.CTkButton):
                widget.configure(
                    fg_color=selected_colors["game_list_btn"], 
                    text_color=selected_colors["button_text"],
                    hover_color=selected_colors["dark_btn_hover"]
                )

def change_app_theme(app, new_theme):
    """
    Modifie le thème de l'application et les couleurs de l'UI.
    """
    ctk.set_appearance_mode(new_theme)

    color_schemes = {
        "Light": {
            "main_bg": "#ffffff",
            "text": "#000000",
            "settings_bg": "#505050",
            "button_fg": "#e5e5e5",
            "button_text": "#000000",
            "user_entry": "#FFFFFF",
            "dark_btn_fg": "#656565",
            "dark_btn_hover": "#848484",
            "header_bg": "#c0c0c0",
            "right_panel_bg": "#ffffff",
            "left_panel_bg": "#e5e5e5",
            "game_list_fg": "#b4b4b4",
            "game_list_btn": "#0C6AA4",
            "settings_logo":"#FFFFFF"
        },
        "Dark": {
            "main_bg": "#353535",
            "text": "#ffffff",
            "settings_bg": "#505050",
            "button_fg": "#505050",
            "button_text": "#ffffff",
            "user_entry": "#818181",
            "dark_btn_fg": "#0e2433",
            "dark_btn_hover": "#8A8787",
            "header_bg": "#1a1a1a",
            "right_panel_bg": "#353535",
            "left_panel_bg": "#636161",
            "game_list_fg": "#333333",
            "game_list_btn": "#1B435C",
            "settings_logo":"#ffffff"
        },
        "Blue": {
            "main_bg": "#ffffff",
            "text": "#000000",
            "settings_bg": "#505050",
            "button_fg": "#e5e5e5",
            "button_text": "#FFFFFF",
            "user_entry": "#FFFFFF",
            "dark_btn_fg": "#0e2433",
            "dark_btn_hover": "#505050",
            "header_bg": "#0e2433",
            "right_panel_bg": "#ffffff",
            "left_panel_bg": "#e5e5e5",
            "game_list_fg": "#d0d0d0",
            "game_list_btn": "#628092",
            "settings_logo":"#ffffff"
        }
    }
    
    selected_colors = color_schemes.get(new_theme, color_schemes["Blue"])
    
    app.settings_menu.configure(fg_color=selected_colors["settings_bg"])
    
    for widget in app.settings_menu.winfo_children():
        if isinstance(widget, ctk.CTkFrame):
            for child in widget.winfo_children():
                if isinstance(child, ctk.CTkLabel):
                    child.configure(text_color=selected_colors["text"])
                elif isinstance(child, ctk.CTkButton):
                    child.configure(fg_color=selected_colors["button_fg"], text_color=selected_colors["button_text"])
                elif isinstance(child, ctk.CTkOptionMenu):
                    child.configure(fg_color=selected_colors["button_fg"], text_color=selected_colors["button_text"])

    try:
        app.configure(fg_color=selected_colors["main_bg"])
        app.header.configure(fg_color=selected_colors["header_bg"])
        app.settings_btn.configure(fg_color=selected_colors["settings_bg"])
        app.main_content_frame.configure(fg_color=selected_colors["main_bg"])
        app.left_panel.configure(fg_color=selected_colors["left_panel_bg"])
        app.right_panel.configure(fg_color=selected_colors["right_panel_bg"])
        app.local_btn.configure(fg_color=selected_colors["dark_btn_fg"], hover_color=selected_colors["dark_btn_hover"], text_color=selected_colors["button_text"])
        app.host_btn.configure(fg_color=selected_colors["dark_btn_fg"], hover_color=selected_colors["dark_btn_hover"], text_color=selected_colors["button_text"])
        app.clear_btn.configure(fg_color=selected_colors["dark_btn_fg"], hover_color=selected_colors["dark_btn_hover"], text_color=selected_colors["button_text"])
        app.connect_btn.configure(fg_color=selected_colors["dark_btn_fg"], hover_color=selected_colors["dark_btn_hover"], text_color=selected_colors["button_text"])
        app.save_btn.configure(fg_color=selected_colors["dark_btn_fg"], hover_color=selected_colors["dark_btn_hover"], text_color=selected_colors["button_text"])
        app.com_connect_btn.configure(fg_color=selected_colors["dark_btn_fg"], hover_color=selected_colors["dark_btn_hover"], text_color=selected_colors["button_text"])
        app.games_list_frame.configure(fg_color=selected_colors["left_panel_bg"])
        #app.connection_state_frame.configure(fg_color=selected_colors["right_panel_bg"])
        app.link_entry.configure(text_color=selected_colors["text"])
        app.connect_label.configure(text_color=selected_colors["text"])
        app.status_label.configure(text_color=selected_colors["text"])
        app.com_connect_btn.configure(fg_color=selected_colors["dark_btn_fg"])
        app.link_entry.configure(fg_color=selected_colors["user_entry"])
        app.arrow_label.configure(text_color=selected_colors["text"])
        app.token_entry.configure(fg_color=selected_colors["user_entry"])
        app.settings_icon_label.configure(text_color=selected_colors["settings_logo"])
        update_game_list_button_colors(app, selected_colors)
    except AttributeError as e:
        print(f"Erreur lors de la mise à jour des couleurs de l'UI : {e}")
        print("Assurez-vous que les widgets sont définis comme des attributs de l'objet 'app' dans la fonction 'create_widgets' pour que les changements de thème fonctionnent correctement.")

    print(f"Thème de l'application changé en : {new_theme}")

def change_board_theme(new_theme):
    """
    Modifie le thème de l'application et les couleurs de l'UI.
    """
       
    if new_theme == "Off":
        send_command(":COLOR SET OFF 0 0 0")
    elif new_theme == "Blue":
        send_command(":COLOR SET BLUE 0 0 100")
    elif new_theme == "Gray":
        send_command(":COLOR SET GRAY 30 30 30")
    elif new_theme == "Green":
        send_command(":COLOR SET GREEN 0 200 0")

    print(f"Thème de l'application changé en : {new_theme}")

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
    app.settings_icon_label = ctk.CTkLabel(menu_content_frame, text="⚙️", font=("Arial", 50, "bold"), text_color="white")
    app.settings_icon_label.pack(pady=(0, 20))

    # Thème de l'application
    app_theme_frame = ctk.CTkFrame(menu_content_frame, fg_color="transparent")
    app_theme_frame.pack(fill="x", pady=10)
    app_theme_frame.columnconfigure(0, weight=1)
    app_theme_frame.columnconfigure(1, weight=1)

    app_theme_option = ctk.CTkOptionMenu(app_theme_frame, values=["Blue", "Dark", "Light"], 
                                             fg_color=c.LEFT_PANEL_BG, text_color="black",
                                             command=lambda new_theme: change_app_theme(app, new_theme))
    app_theme_option.grid(row=0, column=0, sticky="ew", padx=(0, 5))
    app_theme_label = ctk.CTkLabel(app_theme_frame, text="App Theme", text_color="white")
    app_theme_label.grid(row=0, column=1, sticky="w", padx=(5, 0))

    # Thème de l'échiquier
    board_theme_frame = ctk.CTkFrame(menu_content_frame, fg_color="transparent")
    board_theme_frame.pack(fill="x", pady=10)
    board_theme_frame.columnconfigure(0, weight=1)
    board_theme_frame.columnconfigure(1, weight=1)
        
    board_theme_option = ctk.CTkOptionMenu(board_theme_frame, values=["Off", "Blue", "Gray", "Green"],
                                               fg_color=c.LEFT_PANEL_BG, text_color="black",
                                               command=lambda board_theme: change_board_theme(board_theme))
    board_theme_option.grid(row=0, column=0, sticky="ew", padx=(0, 5))
    board_theme_label = ctk.CTkLabel(board_theme_frame, text="Board Theme", text_color="white")
    board_theme_label.grid(row=0, column=1, sticky="w", padx=(5, 0))

    # Slider de luminosité
    brightness_frame = ctk.CTkFrame(menu_content_frame, fg_color="transparent")
    brightness_frame.pack(fill="x", pady=10)
    brightness_frame.columnconfigure(0, weight=1)
    brightness_frame.columnconfigure(1, weight=1)
    
    brightness_slider = ctk.CTkSlider(brightness_frame, from_=0, to=255, command=set_brightness)
    brightness_slider.grid(row=0, column=0, sticky="ew", padx=(0, 5))
    brightness_label = ctk.CTkLabel(brightness_frame, text="Brightness", text_color="white")
    brightness_label.grid(row=0, column=1, sticky="w", padx=(5, 0))

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
