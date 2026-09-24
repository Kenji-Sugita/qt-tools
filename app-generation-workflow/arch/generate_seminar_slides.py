#!/usr/bin/env python3
"""Generate the seminar PDF and remove genpdf's extra blank page."""

from io import BytesIO
from pathlib import Path
import re
import subprocess
import tempfile

from pypdf import PdfReader, PdfWriter
from pypdf.generic import ContentStream
from reportlab.pdfgen import canvas


ARCH_DIR = Path(__file__).resolve().parent
SOURCE = ARCH_DIR / "seminar_slides.md"
OUTPUT = ARCH_DIR / "seminar_slides.pdf"


def page_number_overlay(width: float, height: float, number: int, total: int):
    stream = BytesIO()
    pdf = canvas.Canvas(stream, pagesize=(width, height))
    pdf.setFillColorRGB(1, 1, 1)
    pdf.rect(width - 92, 11, 80, 22, stroke=0, fill=1)
    pdf.setFillColorRGB(0.2, 0.2, 0.2)
    pdf.setFont("Helvetica", 9)
    pdf.drawRightString(width - 20, 20, f"{number} / {total}")
    pdf.save()
    stream.seek(0)
    return PdfReader(stream).pages[0]


def remove_genpdf_page_number(page, reader: PdfReader) -> None:
    content = ContentStream(page.get_contents(), reader)
    end = max(
        index for index, (_, operator) in enumerate(content.operations) if operator == b"ET"
    )
    start = max(
        index
        for index, (_, operator) in enumerate(content.operations[:end])
        if operator == b"BT"
    )
    footer = content.operations[start : end + 1]
    if not any(operator == b"Tj" and operands == ["/"] for operands, operator in footer):
        raise RuntimeError("genpdf page number was not found")
    del content.operations[start : end + 1]
    page.replace_contents(content)


def main() -> None:
    with tempfile.TemporaryDirectory() as temporary_directory:
        generated = Path(temporary_directory) / "seminar_slides.pdf"
        subprocess.run(
            ["genpdf", str(SOURCE), "--output", str(generated)], check=True
        )

        source_pdf = PdfReader(generated)
        if len(source_pdf.pages) < 3:
            raise RuntimeError("genpdf output has too few pages")

        blank_text = source_pdf.pages[1].extract_text().strip()
        if blank_text and not re.fullmatch(r"2 / \d+", blank_text):
            raise RuntimeError("expected blank page was not found at page 2")

        pages = [source_pdf.pages[0], *source_pdf.pages[2:]]
        total = len(pages)
        writer = PdfWriter()
        for number, page in enumerate(pages, start=1):
            writer.add_page(page)
            output_page = writer.pages[-1]
            width = float(output_page.mediabox.width)
            height = float(output_page.mediabox.height)
            remove_genpdf_page_number(output_page, source_pdf)
            output_page.merge_page(page_number_overlay(width, height, number, total))

        with OUTPUT.open("wb") as output_file:
            writer.write(output_file)


if __name__ == "__main__":
    main()
