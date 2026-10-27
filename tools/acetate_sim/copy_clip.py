#!/usr/bin/env python3
"""Copy image file to system clipboard on Linux (X11 / Wayland)."""

import os
import subprocess
import sys


def copy_with_gtk(path: str) -> bool:
    try:
        import gi

        gi.require_version("Gtk", "3.0")
        from gi.repository import Gdk, GdkPixbuf, Gtk

        pixbuf = GdkPixbuf.Pixbuf.new_from_file(path)
        cb = Gtk.Clipboard.get(Gdk.SELECTION_CLIPBOARD)
        cb.set_image(pixbuf)
        cb.store()
        return True
    except Exception:
        return False


def copy_with_cli(path: str) -> bool:
    # Try xclip
    try:
        subprocess.run(
            ["xclip", "-selection", "clipboard", "-t", "image/png", "-i", path],
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        return True
    except Exception:
        pass

    # Try wl-copy
    try:
        with open(path, "rb") as f:
            subprocess.run(
                ["wl-copy", "--type", "image/png"],
                input=f.read(),
                check=True,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            return True
    except Exception:
        pass

    return False


def main():
    if len(sys.argv) < 2:
        return
    path = sys.argv[1]
    if not os.path.isfile(path):
        return

    if copy_with_cli(path):
        return
    copy_with_gtk(path)


if __name__ == "__main__":
    main()
