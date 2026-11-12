#!/usr/bin/env python3
"""Generate the one-page application workflow loop engineering concept slide."""

from pathlib import Path
import subprocess
import tempfile

from pypdf import PdfReader, PdfWriter


DOCS_DIR = Path(__file__).resolve().parent
SOURCE = DOCS_DIR / "app_generation_loop_engineering_concept.md"
OUTPUT = DOCS_DIR / "app_generation_loop_engineering_concept.pdf"


def main() -> None:
    with tempfile.TemporaryDirectory() as temporary_directory:
        generated = Path(temporary_directory) / OUTPUT.name
        subprocess.run(
            ["genpdf", str(SOURCE), "--output", str(generated)], check=True
        )

        source_pdf = PdfReader(generated)
        if len(source_pdf.pages) != 2:
            raise RuntimeError("expected a cover and one content slide")

        writer = PdfWriter()
        writer.add_page(source_pdf.pages[1])
        with OUTPUT.open("wb") as output_file:
            writer.write(output_file)


if __name__ == "__main__":
    main()
