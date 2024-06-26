import tkinter as tk
from tkinter import ttk, messagebox
import os
import subprocess
from pathlib import Path

def refresh_list_view():
    for item in tree.get_children():
        tree.delete(item)

    for entry in os.scandir('.'):
        if entry.is_dir():
            tree.insert('', 'end', text=entry.name, values=('Folder',), image=folder_icon)
        else:
            tree.insert('', 'end', text=entry.name, values=('File',), image=file_icon)

def open_selected_file(event):
    selected_item = tree.selection()
    if selected_item:
        file_name = tree.item(selected_item, 'text')
        try:
            if os.name == 'posix':
                subprocess.call(('xdg-open', file_name))
            else:
                os.startfile(file_name)
        except Exception as e:
            messagebox.showerror("Error", str(e))

def create_new_folder():
    base_name = "NewFolder"
    new_folder_name = base_name
    counter = 1

    while Path(new_folder_name).exists():
        new_folder_name = f"{base_name}{counter}"
        counter += 1

    try:
        os.mkdir(new_folder_name)
        refresh_list_view()
    except Exception as e:
        messagebox.showerror("Error", str(e))

# Create the main window
root = tk.Tk()
root.title("Directory List View")
root.geometry("600x400")
root.configure(bg='yellow')

# Create the treeview with columns
tree = ttk.Treeview(root, columns=('Type',), show='tree headings')
tree.heading('#0', text='Name')
tree.heading('Type', text='Type')

tree.grid(row=0, column=0, sticky='nsew')

# Add a vertical scrollbar to the treeview
vsb = ttk.Scrollbar(root, orient="vertical", command=tree.yview)
vsb.grid(row=0, column=1, sticky='ns')
tree.configure(yscrollcommand=vsb.set)

# Load icons for folders and files
folder_icon = tk.PhotoImage(file='folder.png')  # Use an actual path to a folder icon
file_icon = tk.PhotoImage(file='file.png')      # Use an actual path to a file icon

# Bind double-click event to open file
tree.bind("<Double-1>", open_selected_file)

# Create a button to create a new folder
btn_create_folder = tk.Button(root, text="Create New Folder", command=create_new_folder)
btn_create_folder.grid(row=1, column=0, pady=10)

# Configure row/column weights
root.grid_rowconfigure(0, weight=1)
root.grid_columnconfigure(0, weight=1)

# Populate the treeview with the current directory contents
refresh_list_view()

# Start the main loop
root.mainloop()

