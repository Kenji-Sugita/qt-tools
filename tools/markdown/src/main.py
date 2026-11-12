from __future__ import annotations

import argparse
import base64
import html
import importlib
import mimetypes
import os
import re
import shutil
import subprocess
import sys
import tempfile
from datetime import date
from pathlib import Path


FORMATS = {
    "book": "book-prefix.md",
    "slides": "slides-prefix.md",
}
PAGE_BREAK_MARKER = '<div class="page-break"></div>'
FOOTER_DIRECTIVE_PATTERN = re.compile(r"^!footer\{(?P<text>[^}]*)\}[ \t]*$", flags=re.MULTILINE)
FONT_SIZE_DIRECTIVE_PATTERN = re.compile(r"^!font-size\{(?P<value>[^}]*)\}[ \t]*$", flags=re.MULTILINE)
INCREMENTAL_LIST_DIRECTIVE_PATTERN = re.compile(r"^!incremental-list[ \t]*$", flags=re.MULTILINE)
PAUSE_DIRECTIVE_PATTERN = re.compile(r"^!pause[ \t]*$", flags=re.MULTILINE)
NOTES_DIRECTIVE_INLINE_PATTERN = re.compile(r"^!notes\{(?P<text>[^}]*)\}[ \t]*$")
NOTES_DIRECTIVE_START_PATTERN = re.compile(r"^!notes\{(?P<text>.*)$")
TOC_DIRECTIVE_PATTERN = re.compile(r"^!toc[ \t]*$", flags=re.MULTILINE)
TABLE_SETTING_KEYS = {
    "header_background",
    "header_color",
    "border_color",
    "border_width",
    "stripe",
    "stripe_background",
    "cell_padding",
    "font_size",
    "compact",
    "align",
    "header_align",
    "cell_align",
}
BACKGROUND_SETTING_KEYS = {
    "image",
    "path",
    "file",
    "src",
    "target",
    "size",
    "position",
    "repeat",
    "opacity",
}
SAFE_CSS_VALUE_PATTERN = re.compile(r"^[#%(),./0-9A-Za-z_ -]+$")
FIT_CODE_DIRECTIVE_PATTERN = re.compile(
    r"^!fit-code[ \t]*\n(?P<code>(?P<fence>`{3,}|~{3,})[^\n]*\n.*?\n(?P=fence)[ \t]*\n?)",
    flags=re.MULTILINE | re.DOTALL,
)
MERMAID_CODE_BLOCK_PATTERN = re.compile(
    r"(?ms)^(?P<fence>`{3,}|~{3,})[ \t]*mermaid(?:[ \t]+[^\n]*)?\n.*?\n(?P=fence)[ \t]*$"
)
DOT_CODE_BLOCK_PATTERN = re.compile(
    r"(?ms)^(?:!dot\{(?P<args>[^}]*)\}[ \t]*\n)?(?P<fence>`{3,}|~{3,})[ \t]*dot(?:[ \t]+[^\n]*)?\n(?P<code>.*?)\n(?P=fence)[ \t]*$"
)


def is_fence_line(line: str) -> bool:
    stripped = line.strip()
    return stripped.startswith("```") or stripped.startswith("~~~")


def has_directive_outside_fenced_code(markdown_text: str, *, predicate: callable) -> bool:
    in_fenced_code = False
    for line in markdown_text.splitlines():
        if is_fence_line(line):
            in_fenced_code = not in_fenced_code
            continue
        if not in_fenced_code and predicate(line):
            return True
    return False


def normalize_platform_path(value: str | os.PathLike[str]) -> Path:
    raw = os.fspath(value)
    cygpath = shutil.which("cygpath")

    if os.name != "nt" and re.match(r"^[A-Za-z]:\\", raw):
        if cygpath is not None:
            converted = subprocess.run(
                [cygpath, "-u", raw],
                check=False,
                capture_output=True,
                text=True,
            )
            if converted.returncode == 0:
                raw = converted.stdout.strip()
        else:
            drive = raw[0].lower()
            suffix = raw[2:].replace("\\", "/")
            raw = f"/cygdrive/{drive}{suffix}"

    if os.name == "nt" and raw.startswith("/"):
        if cygpath is not None:
            converted = subprocess.run(
                [cygpath, "-w", raw],
                check=False,
                capture_output=True,
                text=True,
            )
            if converted.returncode == 0:
                raw = converted.stdout.strip()
        else:
            match = re.match(r"^/cygdrive/(?P<drive>[A-Za-z])(?P<suffix>(?:/.*)?)$", raw)
            if match:
                drive = match.group("drive").upper()
                suffix = match.group("suffix").replace("/", "\\")
                raw = f"{drive}:{suffix}"
    return Path(raw)


def normalize_date_argument(argv: list[str]) -> list[str]:
    normalized: list[str] = []
    index = 0
    while index < len(argv):
        token = argv[index]
        if token != "--date":
            normalized.append(token)
            index += 1
            continue

        remaining = argv[index + 1 :]
        non_option_count = sum(1 for value in remaining if not value.startswith("-"))
        normalized.append(f"--date={date.today().isoformat()}" if non_option_count <= 1 else "--date")
        index += 1

    return normalized


def build_arg_parser(*, show_deprecated: bool) -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="genpdf",
        description="Render an existing Markdown file to PDF without editing the source.",
        add_help=False,
    )
    parser.add_argument("-h", action="store_true", help="show short help message and exit")
    parser.add_argument("--help", action="store_true", help="show full help message and exit")
    parser.add_argument("source", nargs="?", help="Path to the source Markdown file.")
    parser.add_argument(
        "--format",
        choices=sorted(FORMATS),
        help="Rendering template to apply.",
    )
    parser.add_argument(
        "--output",
        help="Output PDF path. Defaults to '<source stem>.pdf' beside the source file.",
    )
    deprecated_help = (
        {
            "title": "Deprecated. Use genpdf.title in front matter instead.",
            "subtitle": "Deprecated. Use genpdf.subtitle in front matter instead.",
            "author": "Deprecated. Use genpdf.author in front matter instead.",
            "copyright": "Deprecated. Use genpdf.copyright in front matter instead.",
            "page_numbers": "Deprecated. Use genpdf.page_numbers in front matter instead.",
            "font_size": "Deprecated. Use genpdf.font_size in front matter instead.",
            "date": "Deprecated. Use genpdf.date in front matter instead.",
        }
        if show_deprecated
        else {
            "title": argparse.SUPPRESS,
            "subtitle": argparse.SUPPRESS,
            "author": argparse.SUPPRESS,
            "copyright": argparse.SUPPRESS,
            "page_numbers": argparse.SUPPRESS,
            "font_size": argparse.SUPPRESS,
            "date": argparse.SUPPRESS,
        }
    )
    parser.add_argument("--title", help=deprecated_help["title"])
    parser.add_argument("--subtitle", help=deprecated_help["subtitle"])
    parser.add_argument("--author", help=deprecated_help["author"])
    parser.add_argument("--copyright", help=deprecated_help["copyright"])
    parser.add_argument(
        "--page-numbers",
        dest="page_numbers",
        action=argparse.BooleanOptionalAction,
        default=None,
        help=deprecated_help["page_numbers"],
    )
    parser.add_argument("--font-size", dest="font_size", help=deprecated_help["font_size"])
    parser.add_argument(
        "--date",
        nargs="?",
        const=date.today().isoformat(),
        default=None,
        metavar="DATE",
        help=deprecated_help["date"],
    )
    parser.add_argument(
        "--passthrough",
        action="store_true",
        help="Render the source as-is without prepending a template header.",
    )
    parser.add_argument(
        "--keep-temp",
        action="store_true",
        help="Keep the generated temporary Markdown file for debugging.",
    )
    parser.add_argument(
        "-T",
        "--front-matter-template",
        action="store_true",
        help="Print a complete genpdf front matter template to stdout and exit.",
    )
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    argv = normalize_date_argument(sys.argv[1:] if argv is None else argv)
    if "--help" in argv:
        print(build_arg_parser(show_deprecated=True).format_help(), end="")
        raise SystemExit(0)
    if "-h" in argv:
        print(build_arg_parser(show_deprecated=False).format_help(), end="")
        raise SystemExit(0)

    parser = build_arg_parser(show_deprecated=False)
    args = parser.parse_args(argv)
    if not args.front_matter_template and not args.source:
        parser.error("the following arguments are required: source")
    return args


def warn_deprecated_cli_options(args: argparse.Namespace) -> None:
    deprecated_options = (
        ("title", "--title", "genpdf.title"),
        ("subtitle", "--subtitle", "genpdf.subtitle"),
        ("author", "--author", "genpdf.author"),
        ("copyright", "--copyright", "genpdf.copyright"),
        ("font_size", "--font-size", "genpdf.font_size"),
        ("date", "--date", "genpdf.date"),
    )
    for attr, option, front_matter_key in deprecated_options:
        if getattr(args, attr) is not None:
            print(
                f"Warning: {option} is deprecated. Use {front_matter_key} in front matter instead.",
                file=sys.stderr,
            )
    if args.page_numbers is not None:
        print(
            "Warning: --page-numbers/--no-page-numbers is deprecated. "
            "Use genpdf.page_numbers in front matter instead.",
            file=sys.stderr,
        )


def build_front_matter_template() -> str:
    return f"""---
genpdf:
  format: book
  title: |-
    文書タイトル
    2行目
  subtitle: |-
    サブタイトル
    2行目
  author: 著者名
  version: 1.0
  date: {date.today().isoformat()}
  font_size: 12pt
  page_numbers: true
  copyright: 2026 Example, Inc.
  background:
    image: company-watermark.svg
    target: all
    size: contain
    position: center
    repeat: no-repeat
    opacity: 0.08
  table:
    header_background: "#e8f1ff"
    header_color: "#1f2937"
    border_color: "#cbd5e1"
    border_width: "1px"
    stripe: true
    stripe_background: "#f8fafc"
    cell_padding: "0.45em 0.65em"
    font_size: "0.95em"
    compact: false
    align: left
    header_align: center
    cell_align: left
---
"""


def detect_title(markdown_text: str, fallback: str) -> str:
    for line in markdown_text.splitlines():
        stripped = line.strip()
        if stripped.startswith("# "):
            return stripped[2:].strip()
    return fallback


def yaml_quote(value: str) -> str:
    return "'" + value.replace("'", "''") + "'"


def build_body_font_size_style(font_size: str | None) -> str:
    normalized = (font_size or "").strip()
    if not normalized:
        return ""
    if re.fullmatch(r"\d+(?:\.\d+)?", normalized):
        normalized = f"{normalized}pt"
    return normalized


def data_uri_for_file(path: Path) -> str:
    mime_type = mimetypes.guess_type(path)[0] or "application/octet-stream"
    encoded = base64.b64encode(path.read_bytes()).decode("ascii")
    return f"data:{mime_type};base64,{encoded}"


def inline_svg_local_image_hrefs(path: Path) -> str:
    text = path.read_text(encoding="utf-8")

    def replace_href(match: re.Match[str]) -> str:
        attribute = match.group("attribute")
        quote = match.group("quote")
        href = match.group("href")
        if re.match(r"^[a-zA-Z][a-zA-Z0-9+.-]*:", href) or href.startswith("#"):
            return match.group(0)
        referenced = (path.parent / href).resolve()
        if not referenced.is_file():
            return match.group(0)
        return f"{attribute}={quote}{data_uri_for_file(referenced)}{quote}"

    return re.sub(
        r"(?P<attribute>(?:xlink:)?href)=(?P<quote>[\"'])(?P<href>[^\"']+\.(?:png|jpe?g|gif|webp|svg))(?P=quote)",
        replace_href,
        text,
        flags=re.IGNORECASE,
    )


def strip_yaml_scalar_quotes(value: str) -> str:
    normalized = value.strip()
    if (
        len(normalized) >= 2
        and normalized[0] == normalized[-1]
        and normalized[0] in "\"'"
    ):
        return normalized[1:-1]
    return normalized


def is_yaml_literal_block_scalar(value: str) -> bool:
    return value.strip() in {"|", "|-", "|+"}


def read_yaml_literal_block(lines: list[str], start_index: int, parent_indent: int) -> tuple[str, int]:
    content_lines: list[str] = []
    content_indent: int | None = None
    index = start_index + 1
    while index < len(lines):
        raw_line = lines[index]
        if not raw_line.strip():
            if content_indent is not None:
                content_lines.append("")
            index += 1
            continue

        indent = len(raw_line) - len(raw_line.lstrip(" \t"))
        if indent <= parent_indent:
            break

        if content_indent is None:
            content_indent = indent
        content_lines.append(raw_line[content_indent:])
        index += 1

    return "\n".join(content_lines), index


def escape_cover_text(value: str) -> str:
    return "<br>\n".join(html.escape(line) for line in value.splitlines())


def build_prefix(
    template_dir: Path,
    template_name: str,
    title: str,
    subtitle: str,
    author: str,
    version: str,
    date_text: str,
    font_size: str | None = None,
    *,
    include_cover: bool = True,
    html_prelude: str = "",
) -> str:
    template_path = normalize_platform_path(template_dir / "templates" / template_name)
    template = template_path.read_text(encoding="utf-8")
    markdown_css = normalize_platform_path(template_dir / "styles" / "markdown.css").resolve()
    format_css = normalize_platform_path(
        template_dir / "styles" / ("slides.css" if template_name.startswith("slides") else "book.css")
    ).resolve()
    version_html = f'<div class="version">Version {html.escape(version)}</div>' if version else ""
    template = re.sub(
        r"^(?P<indent>[ \t]*)__VERSION_HTML__[ \t]*\n",
        lambda match: f"{match.group('indent')}{version_html}\n" if version_html else "",
        template,
        flags=re.MULTILINE,
    )
    replacements = {
        "__MARKDOWN_CSS__": yaml_quote(str(markdown_css)),
        "__FORMAT_CSS__": yaml_quote(str(format_css)),
        "__TITLE_HTML__": escape_cover_text(title),
        "__SUBTITLE_HTML__": escape_cover_text(subtitle),
        "__AUTHOR_HTML__": html.escape(author),
        "__DATE_HTML__": html.escape(date_text),
    }
    for key, value in replacements.items():
        template = template.replace(key, value)
    if not include_cover:
        match = re.match(r"^(---\n.*?\n---\n?)", template, flags=re.DOTALL)
        if match:
            template = match.group(1)
    if html_prelude.strip():
        template = template.rstrip() + "\n\n" + html_prelude.strip() + "\n"
    return template.rstrip() + "\n\n"


def has_mermaid_code_block(markdown_text: str) -> bool:
    return MERMAID_CODE_BLOCK_PATTERN.search(markdown_text) is not None


def has_dot_code_block(markdown_text: str) -> bool:
    return DOT_CODE_BLOCK_PATTERN.search(markdown_text) is not None


def parse_directive_args(raw_args: str | None) -> dict[str, str]:
    args: dict[str, str] = {}
    for token in (raw_args or "").split():
        if "=" not in token:
            continue
        key, value = token.split("=", 1)
        args[key.strip().lower()] = value.strip()
    return args


def scale_svg_dimensions(svg: str, scale: float) -> str:
    if scale == 1:
        return svg

    def scale_match(match: re.Match[str]) -> str:
        value = float(match.group("value")) * scale
        unit = match.group("unit") or ""
        return f'{match.group("attr")}="{value:.3f}{unit}"'

    return re.sub(
        r'(?P<attr>\b(?:width|height))="(?P<value>\d+(?:\.\d+)?)(?P<unit>[A-Za-z%]*)"',
        scale_match,
        svg,
        count=2,
    )


def render_dot_svg(dot_code: str, *, scale: float = 1) -> str:
    dot_command = shutil.which("dot")
    if dot_command is None:
        raise RuntimeError(
            "Graphviz `dot` is required for ```dot diagrams. Install Graphviz so that `dot` is available on PATH."
        )

    completed = subprocess.run(
        [dot_command, "-Tsvg"],
        input=dot_code,
        capture_output=True,
        text=True,
        check=False,
    )
    if completed.returncode != 0:
        message = completed.stderr.strip() or completed.stdout.strip() or "unknown Graphviz error"
        raise RuntimeError(f"Failed to render Graphviz DOT diagram:\n{message}")

    svg = completed.stdout.strip()
    svg = re.sub(r"^\s*<\?xml[^>]*>\s*", "", svg)
    svg = re.sub(r"^\s*<!DOCTYPE[^>]*(?:\[[\s\S]*?\]\s*)?>\s*", "", svg)
    return scale_svg_dimensions(svg.strip(), scale)


def expand_dot_diagrams(markdown_text: str) -> str:
    def replacement(match: re.Match[str]) -> str:
        args = parse_directive_args(match.group("args"))
        requested_align = args.get("align", "center").lower()
        align = requested_align if requested_align in {"left", "center", "right"} else "center"
        try:
            scale = float(args.get("scale", "1"))
        except ValueError:
            scale = 1.0
        if scale <= 0:
            scale = 1.0

        svg = render_dot_svg(match.group("code").strip(), scale=scale)
        return (
            f'<div class="dot-diagram dot-align-{align}" '
            f'style="--dot-scale: {html.escape(str(scale))}">\n'
            f"{svg}\n"
            "</div>"
        )

    return DOT_CODE_BLOCK_PATTERN.sub(replacement, markdown_text)


def resolve_mermaid_bundle_path(template_dir: Path) -> Path | None:
    candidates = [
        Path.cwd() / "node_modules" / "mermaid" / "dist" / "mermaid.min.js",
        Path.cwd() / "node_modules" / "mermaid" / "dist" / "mermaid.js",
        template_dir.parent.parent / "node_modules" / "mermaid" / "dist" / "mermaid.min.js",
        template_dir.parent.parent / "node_modules" / "mermaid" / "dist" / "mermaid.js",
    ]

    npm_cache = Path.home() / ".npm" / "_npx"
    cached_candidates = sorted(
        list(npm_cache.glob("*/node_modules/mermaid/dist/mermaid.min.js"))
        + list(npm_cache.glob("*/node_modules/mermaid/dist/mermaid.js")),
        key=lambda path: path.stat().st_mtime,
        reverse=True,
    )
    candidates.extend(cached_candidates)

    for candidate in candidates:
        if candidate.is_file():
            return candidate.resolve()

    return None


def build_mermaid_html_prelude(mermaid_bundle_path: Path | None) -> str:
    script_src = (
        mermaid_bundle_path.as_uri()
        if mermaid_bundle_path is not None
        else "https://cdn.jsdelivr.net/npm/mermaid@11/dist/mermaid.min.js"
    )
    escaped_src = html.escape(script_src, quote=True)
    return f"""
<style>
.mermaid {{
  margin: 1em 0 1.25em;
  text-align: center;
}}

.mermaid svg {{
  display: block;
  width: auto !important;
  max-width: 100%;
  height: auto;
  margin-left: auto;
  margin-right: auto;
}}

.mermaid-align-left svg {{
  margin-left: 0;
  margin-right: auto;
}}

.mermaid-align-center svg {{
  margin-left: auto;
  margin-right: auto;
}}

.mermaid-align-right svg {{
  margin-left: auto;
  margin-right: 0;
}}
</style>
<script src="{escaped_src}"></script>
<script>
(function() {{
  const render = async () => {{
    if (!window.mermaid) {{
      return;
    }}

    const mermaidApi = window.mermaid.default || window.mermaid;
    mermaidApi.initialize({{
      startOnLoad: false,
      securityLevel: "loose",
      theme: "default"
    }});

    const nodes = Array.from(document.querySelectorAll(".mermaid"));
    if (!nodes.length) {{
      return;
    }}

    try {{
      await mermaidApi.run({{ nodes }});
      for (const node of nodes) {{
        const svg = node.querySelector("svg");
        if (!svg) {{
          continue;
        }}

        const scaleValue = Number.parseFloat(getComputedStyle(node).getPropertyValue("--mermaid-scale"));
        const scale = Number.isFinite(scaleValue) && scaleValue > 0 ? scaleValue : 1;
        const viewBox = svg.viewBox && svg.viewBox.baseVal ? svg.viewBox.baseVal : null;
        const attrWidth = Number.parseFloat(svg.getAttribute("width") || "");
        const attrHeight = Number.parseFloat(svg.getAttribute("height") || "");
        const baseWidth = viewBox && viewBox.width ? viewBox.width : (Number.isFinite(attrWidth) ? attrWidth : svg.getBoundingClientRect().width);
        const baseHeight = viewBox && viewBox.height ? viewBox.height : (Number.isFinite(attrHeight) ? attrHeight : svg.getBoundingClientRect().height);

        if (Number.isFinite(baseWidth) && baseWidth > 0) {{
          svg.style.width = `${{baseWidth * scale}}px`;
        }}
        if (Number.isFinite(baseHeight) && baseHeight > 0) {{
          svg.style.height = `${{baseHeight * scale}}px`;
        }}
        svg.style.maxWidth = "100%";
      }}
    }} catch (error) {{
      console.error("Failed to render Mermaid diagrams:", error);
    }}
  }};

  if (document.readyState === "loading") {{
    document.addEventListener("DOMContentLoaded", () => {{
      void render();
    }}, {{ once: true }});
  }} else {{
    void render();
  }}
}})();
</script>
"""


def sanitize_css_value(value: str | None) -> str:
    normalized = strip_yaml_scalar_quotes(value or "")
    if not normalized or not SAFE_CSS_VALUE_PATTERN.fullmatch(normalized):
        return ""
    return normalized


def sanitize_background_opacity(value: str | None) -> str:
    normalized = strip_yaml_scalar_quotes(value or "")
    if not normalized:
        return "0.08"
    try:
        opacity = float(normalized)
    except ValueError:
        return "0.08"
    if opacity < 0:
        opacity = 0
    if opacity > 1:
        opacity = 1
    return f"{opacity:g}"


def sanitize_background_repeat(value: str | None) -> str:
    normalized = strip_yaml_scalar_quotes(value or "").lower()
    return normalized if normalized in {"repeat", "no-repeat", "repeat-x", "repeat-y"} else "no-repeat"


def collect_background_settings(settings: dict[str, str]) -> dict[str, str]:
    return {
        key.removeprefix("background."): value
        for key, value in settings.items()
        if key.startswith("background.")
    }


def resolve_background_image(background_settings: dict[str, str], source_dir: Path) -> tuple[Path, str]:
    image = (
        background_settings.get("image")
        or background_settings.get("path")
        or background_settings.get("file")
        or background_settings.get("src")
    )
    image_file_name = strip_yaml_scalar_quotes(image or "")
    if not image_file_name:
        raise ValueError("`genpdf.background.image` is required when `genpdf.background` is specified.")
    if (
        re.match(r"^[a-zA-Z][a-zA-Z0-9+.-]*:", image_file_name)
        or "/" in image_file_name
        or "\\" in image_file_name
        or Path(image_file_name).name != image_file_name
    ):
        raise ValueError(
            "`genpdf.background.image` must be a file name under background-images/, not a path."
        )

    image_path = (Path(__file__).resolve().parent.parent / "background-images" / image_file_name).resolve()
    if not image_path.is_file():
        raise FileNotFoundError(f"Background image file not found: {image_path}")
    return image_path, data_uri_for_file(image_path)


def split_background_size(value: str | None) -> tuple[str, str | None]:
    size_parts = strip_yaml_scalar_quotes(value or "").split()
    width = sanitize_css_value(size_parts[0]) if size_parts else ""
    height = sanitize_css_value(size_parts[1]) if len(size_parts) >= 2 else ""
    return width, height or None


def css_intrinsic_svg_height(image_path: Path) -> str:
    if image_path.suffix.lower() != ".svg":
        return ""
    match = re.search(
        r"<svg\b[^>]*\bheight=[\"'](?P<height>\d+(?:\.\d+)?(?:mm|cm|in|px|pt|pc)?)[\"']",
        image_path.read_text(encoding="utf-8", errors="ignore"),
        flags=re.IGNORECASE,
    )
    return sanitize_css_value(match.group("height")) if match else ""


def expand_css_length_for_footer_margin(value: str) -> str:
    match = re.fullmatch(r"(?P<number>\d+(?:\.\d+)?)(?P<unit>mm|cm|in|px|pt|pc)", value)
    if not match:
        return value
    number = float(match.group("number"))
    unit = match.group("unit")
    increments = {
        "mm": 1.0,
        "cm": 0.1,
        "in": 0.04,
        "px": 4.0,
        "pt": 3.0,
        "pc": 0.25,
    }
    return f"{number + increments[unit]:g}{unit}"


def build_background_footer_env(settings: dict[str, str], source_dir: Path) -> dict[str, str]:
    return {}


def has_bottom_background(settings: dict[str, str]) -> bool:
    background_settings = collect_background_settings(settings)
    raw_position = strip_yaml_scalar_quotes(background_settings.get("position") or "")
    return bool(background_settings) and "bottom" in raw_position.lower()


def apply_background_footer_margin(markdown_text: str, bottom_margin: str | None) -> str:
    if not bottom_margin:
        return markdown_text
    replacements = (
        (r"(?m)^  margin: 10mm 0 15mm 0$", f"  margin: 10mm 0 {bottom_margin} 0"),
        (r"(?m)^  margin: 30mm 20mm$", f"  margin: 30mm 20mm {bottom_margin} 20mm"),
    )
    for pattern, replacement in replacements:
        updated, count = re.subn(pattern, replacement, markdown_text, count=1)
        if count:
            return updated
    return markdown_text


def subtract_css_lengths(left: str, right: str) -> str:
    match_left = re.fullmatch(r"(?P<number>\d+(?:\.\d+)?)(?P<unit>mm|cm|in|px|pt|pc)", left)
    match_right = re.fullmatch(r"(?P<number>\d+(?:\.\d+)?)(?P<unit>mm|cm|in|px|pt|pc)", right)
    if not match_left or not match_right or match_left.group("unit") != match_right.group("unit"):
        raise ValueError(f"Cannot subtract CSS lengths: {left} - {right}")
    return f"{float(match_left.group('number')) - float(match_right.group('number')):g}{match_left.group('unit')}"


def css_length_to_pt(value: str, reference_pt: float | None = None) -> float:
    match = re.fullmatch(r"(?P<number>\d+(?:\.\d+)?)(?P<unit>mm|cm|in|px|pt|pc|%)", value)
    if not match:
        raise ValueError(f"Unsupported CSS length: {value}")
    number = float(match.group("number"))
    unit = match.group("unit")
    if unit == "%":
        if reference_pt is None:
            raise ValueError(f"Cannot resolve percentage CSS length without reference: {value}")
        return reference_pt * number / 100
    factors = {
        "mm": 72 / 25.4,
        "cm": 72 / 2.54,
        "in": 72,
        "px": 0.75,
        "pt": 1,
        "pc": 12,
    }
    return number * factors[unit]


def format_pt(value: float) -> str:
    return f"{value:.4f}".rstrip("0").rstrip(".") + "pt"


def page_size_for_format(format_name: str) -> tuple[str, str]:
    return ("297mm", "210mm") if format_name == "slides" else ("210mm", "297mm")


def apply_bottom_background_to_pdf(
    output: Path,
    settings: dict[str, str],
    source_dir: Path,
    format_name: str,
    page_numbers: bool,
) -> None:
    background_settings = collect_background_settings(settings)
    if not background_settings:
        return

    raw_position = strip_yaml_scalar_quotes(background_settings.get("position") or "")
    if "bottom" not in raw_position.lower():
        return

    if shutil.which("rsvg-convert") is None:
        raise RuntimeError("Bottom background SVG rendering requires the `rsvg-convert` command.")

    target = strip_yaml_scalar_quotes(background_settings.get("target") or "all").lower()
    if target != "all":
        raise ValueError("Only `genpdf.background.target: all` is currently supported.")

    image_path, data_uri = resolve_background_image(background_settings, source_dir)
    size_width, bottom_height = split_background_size(background_settings.get("size"))
    image_height = css_intrinsic_svg_height(image_path) or bottom_height or "15mm"

    temp_dir = Path(tempfile.mkdtemp(prefix=f"{output.stem}-background-"))
    try:
        if image_path.suffix.lower() == ".svg":
            background_svg = temp_dir / image_path.name
            background_svg.write_text(inline_svg_local_image_hrefs(image_path), encoding="utf-8")
            image_href = background_svg.as_uri()
        else:
            image_href = data_uri

        pypdf = importlib.import_module("pypdf")
        reader = pypdf.PdfReader(str(output))
        writer = pypdf.PdfWriter()
        total_pages = len(reader.pages)

        for index, page in enumerate(reader.pages, start=1):
            page_width_pt = float(page.mediabox.width)
            page_height_pt = float(page.mediabox.height)
            image_height_pt = css_length_to_pt(image_height, page_height_pt)
            clip_height_pt = css_length_to_pt(bottom_height, page_height_pt) if bottom_height else image_height_pt
            image_y_pt = page_height_pt - image_height_pt
            clip_y_pt = page_height_pt - clip_height_pt
            render_full_width = (size_width or "100%") == "100%"
            render_bleed_pt = 2.0 if render_full_width else 0.0
            render_x_pt = -render_bleed_pt if render_full_width else 0.0
            render_width_pt = (
                page_width_pt + (render_bleed_pt * 2)
                if render_full_width
                else css_length_to_pt(size_width, page_width_pt)
            )
            clip_x_pt = -render_bleed_pt if render_full_width else 0.0
            clip_width_pt = page_width_pt + (render_bleed_pt * 2) if render_full_width else page_width_pt
            page_number_x_pt = page_width_pt - css_length_to_pt("18mm")
            page_number_y_pt = page_height_pt - css_length_to_pt("5mm")
            overlay_svg = temp_dir / f"overlay-{index}.svg"
            overlay_pdf = temp_dir / f"overlay-{index}.pdf"
            page_number_svg = (
                f'<text x="{format_pt(page_number_x_pt)}" y="{format_pt(page_number_y_pt)}" font-family="Helvetica, Arial, sans-serif" '
                f'font-size="10pt" fill="#444" text-anchor="end">{index} / {total_pages}</text>'
                if page_numbers
                else ""
            )
            overlay_svg.write_text(
                f'''<svg xmlns="http://www.w3.org/2000/svg" width="{format_pt(page_width_pt)}" height="{format_pt(page_height_pt)}">
  <defs>
    <clipPath id="background-clip">
      <rect x="{format_pt(clip_x_pt)}" y="{format_pt(clip_y_pt)}" width="{format_pt(clip_width_pt)}" height="{format_pt(clip_height_pt)}"/>
    </clipPath>
  </defs>
  <g clip-path="url(#background-clip)" opacity="{sanitize_background_opacity(background_settings.get("opacity"))}">
    <image href="{image_href}" x="{format_pt(render_x_pt)}" y="{format_pt(image_y_pt)}" width="{format_pt(render_width_pt)}" height="{format_pt(image_height_pt)}" preserveAspectRatio="none"/>
  </g>
  {page_number_svg}
</svg>
''',
                encoding="utf-8",
            )
            subprocess.run(
                [
                    "rsvg-convert",
                    "-f",
                    "pdf",
                    "-d",
                    "300",
                    "-p",
                    "300",
                    "-o",
                    str(overlay_pdf),
                    str(overlay_svg),
                ],
                check=True,
            )
            overlay_reader = pypdf.PdfReader(str(overlay_pdf))
            page.merge_page(overlay_reader.pages[0], over=True)
            writer.add_page(page)

        with output.open("wb") as stream:
            writer.write(stream)
    finally:
        for path in temp_dir.glob("*"):
            path.unlink()
        if temp_dir.exists():
            temp_dir.rmdir()


def build_background_html_prelude(settings: dict[str, str], source_dir: Path, format_name: str) -> str:
    background_settings = collect_background_settings(settings)
    if not background_settings:
        return ""

    target = strip_yaml_scalar_quotes(background_settings.get("target") or "all").lower()
    if target != "all":
        raise ValueError("Only `genpdf.background.target: all` is currently supported.")

    raw_position = strip_yaml_scalar_quotes(background_settings.get("position") or "")
    if "bottom" in raw_position.lower():
        return ""

    position = sanitize_css_value(raw_position) or "center"
    repeat = sanitize_background_repeat(background_settings.get("repeat"))
    opacity = sanitize_background_opacity(background_settings.get("opacity"))
    _image_path, data_uri = resolve_background_image(background_settings, source_dir)
    layer_left = "0"
    layer_width = "calc(297mm + 20mm)" if format_name == "slides" else "calc(210mm + 60mm)"
    background_size = sanitize_css_value(background_settings.get("size")) or "contain"
    vertical_position = position
    height = "100vh"
    top_or_bottom = "top: 0;"

    return f"""<style>
body::before {{
  content: "";
  position: fixed;
  {top_or_bottom}
  left: {layer_left};
  width: {layer_width};
  height: {height};
  background-image: url("{data_uri}");
  background-repeat: {repeat};
  background-position: {vertical_position};
  background-size: {background_size};
  opacity: {opacity};
  pointer-events: none;
  z-index: 0;
  overflow: hidden;
}}

.page {{
  background-color: transparent !important;
}}
</style>
"""


def sanitize_table_align(value: str | None) -> str:
    normalized = strip_yaml_scalar_quotes(value or "").lower()
    return normalized if normalized in {"left", "center", "right"} else ""


def build_table_html_prelude(settings: dict[str, str]) -> str:
    table_settings = {
        key.removeprefix("table."): value
        for key, value in settings.items()
        if key.startswith("table.")
    }
    if not table_settings:
        return ""

    compact = parse_bool_setting(table_settings.get("compact"))
    stripe_configured = "stripe" in table_settings
    stripe = parse_bool_setting(table_settings.get("stripe"), default=True)
    header_background = sanitize_css_value(table_settings.get("header_background"))
    header_color = sanitize_css_value(table_settings.get("header_color"))
    border_color = sanitize_css_value(table_settings.get("border_color"))
    border_width = sanitize_css_value(table_settings.get("border_width"))
    stripe_background = sanitize_css_value(table_settings.get("stripe_background"))
    cell_padding = sanitize_css_value(table_settings.get("cell_padding"))
    font_size = sanitize_css_value(table_settings.get("font_size"))
    align = sanitize_table_align(table_settings.get("align"))
    header_align = sanitize_table_align(table_settings.get("header_align")) or align
    cell_align = sanitize_table_align(table_settings.get("cell_align")) or align

    if compact:
        cell_padding = cell_padding or "0.3em 0.55em"
        font_size = font_size or "0.92em"

    rules: list[str] = []
    if font_size:
        rules.append(f"table {{\n  font-size: {font_size};\n}}")
        rules.append(
            "html body table th,\n"
            "html body table td {\n"
            f"  font-size: {font_size} !important;\n"
            "}"
        )
    if border_color or border_width or cell_padding:
        declarations: list[str] = []
        if cell_padding:
            declarations.append(f"  padding: {cell_padding};")
        if border_color or border_width:
            declarations.append(f"  border: {border_width or '1px'} solid {border_color or 'gainsboro'};")
        rules.append("table th,\ntable td {\n" + "\n".join(declarations) + "\n}")
    if header_background or header_color or header_align:
        declarations = []
        if header_background:
            declarations.append(f"  background-color: {header_background};")
        if header_color:
            declarations.append(f"  color: {header_color};")
        if header_align:
            declarations.append(f"  text-align: {header_align};")
        rules.append("table th {\n" + "\n".join(declarations) + "\n}")
    if cell_align:
        rules.append(f"table td {{\n  text-align: {cell_align};\n}}")
    if border_color:
        rules.append(f"table tr {{\n  border-top-color: {border_color};\n}}")
    if stripe_configured or stripe_background:
        stripe_color = stripe_background or "#f8fafc"
        rules.append(
            "table tr:nth-child(2n) {\n"
            f"  background-color: {stripe_color if stripe else 'white'};\n"
            "}"
        )

    if not rules:
        return ""
    return "<style>\n" + "\n\n".join(rules) + "\n</style>\n"


def resolve_md_to_pdf_command(template_dir: Path) -> list[str]:
    node_executable = shutil.which("node") or shutil.which("node.exe")
    npx_executable = (
        shutil.which("npx")
        or shutil.which("npx.cmd")
        or shutil.which("npx.exe")
    )

    local_cli = template_dir / "node_modules" / "md-to-pdf" / "dist" / "cli.js"
    if local_cli.is_file() and node_executable is not None:
        return [node_executable, str(local_cli)]

    npm_cache = Path.home() / ".npm" / "_npx"
    cached_candidates = sorted(
        npm_cache.glob("*/node_modules/md-to-pdf/dist/cli.js"),
        key=lambda path: path.stat().st_mtime,
        reverse=True,
    )
    for candidate in cached_candidates:
        if candidate.is_file() and node_executable is not None:
            return [node_executable, str(candidate)]

    if npx_executable is not None:
        return [npx_executable, "--yes", "md-to-pdf"]

    return ["npx", "--yes", "md-to-pdf"]


def validate_runtime_requirements(template_dir: Path, *, requires_pdf_merge: bool, requires_dot: bool = False) -> None:
    missing_messages: list[str] = []
    command = resolve_md_to_pdf_command(template_dir)
    executable = command[0]

    if shutil.which(executable) is None:
        if Path(executable).name.lower() in {"node", "node.exe"}:
            missing_messages.append(
                "Node.js is required. Install Node.js so that `node` is available on PATH."
            )
        elif Path(executable).name.lower() in {"npx", "npx.cmd", "npx.exe"}:
            missing_messages.append(
                "md-to-pdf could not be launched because `npx` is not available. Install Node.js or install `md-to-pdf` locally."
            )
            missing_messages.append(
                "Alternatively, install `md-to-pdf` in this project with `npm install md-to-pdf`."
            )

    if requires_pdf_merge and importlib.util.find_spec("pypdf") is None:
        missing_messages.append(
            "`!footer{...}` requires the Python package `pypdf`. Install it with `python -m pip install pypdf`."
        )

    if requires_dot and shutil.which("dot") is None:
        missing_messages.append(
            "Graphviz `dot` is required for ```dot diagrams. Install Graphviz so that `dot` is available on PATH."
        )

    if missing_messages:
        raise RuntimeError("Missing runtime requirements:\n- " + "\n- ".join(dict.fromkeys(missing_messages)))


def parse_leading_front_matter(markdown_text: str) -> tuple[dict[str, str], str]:
    markdown_text = markdown_text.removeprefix("\ufeff")

    if not markdown_text.startswith("---\n"):
        return {}, markdown_text

    match = re.match(r"^---\n(?P<body>.*?)\n---\n?", markdown_text, flags=re.DOTALL)
    if not match:
        return {}, markdown_text

    settings: dict[str, str] = {}
    header_settings: dict[str, str] = {}
    inside_genpdf = False
    inside_table = False
    inside_background = False
    table_indent: int | None = None
    background_indent: int | None = None
    lines = match.group("body").splitlines()
    index = 0
    while index < len(lines):
        raw_line = lines[index]
        next_index = index + 1
        if raw_line.startswith((" ", "\t")):
            if not inside_genpdf:
                index = next_index
                continue

            indent = len(raw_line) - len(raw_line.lstrip(" \t"))
            line = raw_line.strip()
            if not line or ":" not in line:
                index = next_index
                continue

            key, value = line.split(":", 1)
            normalized_key = key.strip().lower()
            normalized_value = strip_yaml_scalar_quotes(value)
            if inside_table and table_indent is not None and indent <= table_indent:
                inside_table = False
                table_indent = None
            if inside_background and background_indent is not None and indent <= background_indent:
                inside_background = False
                background_indent = None
            if inside_table and normalized_key in TABLE_SETTING_KEYS:
                settings[f"table.{normalized_key}"] = normalized_value
                index = next_index
                continue
            if inside_background and normalized_key in BACKGROUND_SETTING_KEYS:
                settings[f"background.{normalized_key}"] = normalized_value
                index = next_index
                continue
            if normalized_key == "table" and not normalized_value:
                inside_table = True
                inside_background = False
                table_indent = indent
                background_indent = None
                index = next_index
                continue
            if normalized_key == "background" and not normalized_value:
                inside_background = True
                inside_table = False
                background_indent = indent
                table_indent = None
                index = next_index
                continue
        else:
            line = raw_line.strip()
            if not line or ":" not in line:
                inside_genpdf = False
                inside_table = False
                inside_background = False
                table_indent = None
                background_indent = None
                index = next_index
                continue

            key, value = line.split(":", 1)
            normalized_key = key.strip().lower()
            inside_genpdf = normalized_key == "genpdf"
            inside_table = False
            inside_background = False
            table_indent = None
            background_indent = None
            if inside_genpdf:
                index = next_index
                continue

            if normalized_key != "copyright":
                index = next_index
                continue

            normalized_value = strip_yaml_scalar_quotes(value)
            header_settings[normalized_key] = normalized_value
            index = next_index
            continue

        key, value = line.split(":", 1)
        normalized_key = key.strip().lower()
        if normalized_key not in {"format", "title", "subtitle", "author", "version", "copyright", "date", "page_numbers", "font_size"}:
            index = next_index
            continue

        inside_table = False
        inside_background = False
        normalized_value = strip_yaml_scalar_quotes(value)
        if normalized_key in {"title", "subtitle"} and is_yaml_literal_block_scalar(normalized_value):
            normalized_value, next_index = read_yaml_literal_block(lines, index, indent)
        settings[normalized_key] = normalized_value
        index = next_index

    return header_settings | settings, markdown_text[match.end() :]


def extract_local_reference_paths(markdown_text: str, source_dir: Path) -> list[Path]:
    references: list[Path] = []
    patterns = (
        re.compile(r"!\[[^\]]*\]\((?P<path>[^)\s]+)"),
        re.compile(r"""<img\b[^>]*\bsrc=["'](?P<path>[^"']+)["']""", flags=re.IGNORECASE),
        re.compile(r"!include-code\{[^}]*\bpath=(?P<path>\S+)"),
        re.compile(r"!cover-image\{[^}]*\b(?:path|file|src)=(?P<path>\S+)"),
        re.compile(r"!svg\{[^}]*\b(?:path|file|src)=(?P<path>\S+)"),
    )

    for pattern in patterns:
        for match in pattern.finditer(markdown_text):
            raw_path = match.group("path").strip()
            if (
                not raw_path
                or raw_path.startswith(("#", "data:"))
                or re.match(r"^[a-zA-Z][a-zA-Z0-9+.-]*:", raw_path)
            ):
                continue
            references.append((source_dir / raw_path).resolve())

    return references


def determine_pdf_basedir(source: Path, markdown_text: str) -> Path:
    candidate_paths = [source.parent, *extract_local_reference_paths(markdown_text, source.parent)]
    common_root = Path(os.path.commonpath([str(path) for path in candidate_paths]))
    if common_root.is_file():
        return common_root.parent
    return common_root


def extract_footer_text(markdown_text: str) -> tuple[str | None, str]:
    footer_text: str | None = None
    cleaned_lines: list[str] = []
    in_fenced_code = False

    for line in markdown_text.splitlines():
        if is_fence_line(line):
            in_fenced_code = not in_fenced_code
            cleaned_lines.append(line)
            continue

        if not in_fenced_code:
            match = re.match(r"^!footer\{(?P<text>[^}]*)\}[ \t]*$", line)
            if match:
                footer_text = match.group("text").strip() or None
                continue

        cleaned_lines.append(line)

    if footer_text is None:
        return None, markdown_text

    cleaned = "\n".join(cleaned_lines)
    cleaned = re.sub(r"\n{3,}", "\n\n", cleaned)
    return footer_text, cleaned.strip() + "\n"


def extract_page_font_size(markdown_text: str) -> tuple[str | None, str]:
    page_font_size: str | None = None
    cleaned_lines: list[str] = []
    in_fenced_code = False

    for line in markdown_text.splitlines():
        if is_fence_line(line):
            in_fenced_code = not in_fenced_code
            cleaned_lines.append(line)
            continue

        if not in_fenced_code:
            match = FONT_SIZE_DIRECTIVE_PATTERN.match(line)
            if match:
                page_font_size = match.group("value").strip() or None
                continue

        cleaned_lines.append(line)

    if page_font_size is None:
        return None, markdown_text

    cleaned = "\n".join(cleaned_lines)
    cleaned = re.sub(r"\n{3,}", "\n\n", cleaned)
    return page_font_size, cleaned.strip() + "\n"


def normalize_copyright_text(value: str | None) -> str | None:
    if value is None:
        return None

    normalized = value.strip()
    if not normalized:
        return None

    if normalized.startswith("©") or re.match(r"(?i)^(copyright|\(c\))\b", normalized):
        return normalized

    return f"© {normalized}"


def parse_bool_setting(value: str | None, default: bool = False) -> bool:
    if value is None:
        return default

    normalized = value.strip().lower()
    if normalized in {"true", "yes", "on", "1"}:
        return True
    if normalized in {"false", "no", "off", "0", ""}:
        return False
    return default


def extract_notes(markdown_text: str) -> tuple[list[list[str]], str]:
    page_notes: list[list[str]] = []
    cleaned_pages: list[str] = []

    for page in split_pages(markdown_text):
        notes: list[str] = []
        cleaned_lines: list[str] = []
        in_fenced_code = False
        collecting_note = False
        note_lines: list[str] = []

        for line in page.splitlines():
            if collecting_note:
                if line.strip() == "}":
                    text = "\n".join(note_lines).strip()
                    if text:
                        notes.append(text)
                    collecting_note = False
                    note_lines = []
                    continue
                note_lines.append(line)
                continue

            if is_fence_line(line):
                in_fenced_code = not in_fenced_code
                cleaned_lines.append(line)
                continue

            if not in_fenced_code:
                inline_match = NOTES_DIRECTIVE_INLINE_PATTERN.match(line)
                if inline_match:
                    text = inline_match.group("text").strip()
                    if text:
                        notes.append(text)
                    continue

                start_match = NOTES_DIRECTIVE_START_PATTERN.match(line)
                if start_match:
                    remainder = start_match.group("text")
                    if remainder.endswith("}"):
                        text = remainder[:-1].strip()
                        if text:
                            notes.append(text)
                        continue

                    collecting_note = True
                    note_lines = [remainder] if remainder else []
                    continue

            cleaned_lines.append(line)

        if collecting_note:
            cleaned_lines.append("!notes{")
            cleaned_lines.extend(note_lines)

        cleaned = re.sub(r"\n{3,}", "\n\n", "\n".join(cleaned_lines))
        cleaned_pages.append(cleaned.strip() + "\n")
        page_notes.append(notes)

    return page_notes, join_pages(cleaned_pages)


def write_notes_file(notes_output: Path, page_notes: list[list[str]]) -> None:
    pages_with_notes = [(index + 1, notes) for index, notes in enumerate(page_notes) if notes]
    if not pages_with_notes:
        if notes_output.exists():
            notes_output.unlink()
        return

    lines = ["# Speaker Notes", ""]
    for page_number, notes in pages_with_notes:
        lines.append(f"## Page {page_number}")
        lines.append("")
        for note in notes:
            note_lines = note.splitlines()
            continuation_lines = note_lines[1:]
            continuation_indent = min(
                (
                    len(line) - len(line.lstrip())
                    for line in continuation_lines
                    if line.strip()
                ),
                default=0,
            )
            lines.append(f"- {note_lines[0]}")
            for continuation in continuation_lines:
                if continuation_indent and continuation.strip():
                    continuation = continuation[continuation_indent:]
                lines.append(f"  {continuation}" if continuation else "  ")
        lines.append("")

    notes_output.write_text("\n".join(lines).rstrip() + "\n", encoding="utf-8")


def collect_headings(markdown_text: str) -> list[tuple[int, str]]:
    headings: list[tuple[int, str]] = []
    in_fenced_code = False

    for line in markdown_text.splitlines():
        stripped = line.strip()
        if stripped.startswith("```") or stripped.startswith("~~~"):
            in_fenced_code = not in_fenced_code
            continue
        if in_fenced_code or stripped.startswith("<!--"):
            continue

        match = re.match(r"^(?P<marks>#{1,6})[ \t]+(?P<title>.+?)?[ \t]*#*[ \t]*$", line)
        if not match:
            continue

        title = (match.group("title") or "").strip()
        if not title:
            continue
        headings.append((len(match.group("marks")), title))

    return headings


def build_toc(markdown_text: str) -> str:
    source_lines: list[str] = []
    in_fenced_code = False
    for line in markdown_text.splitlines():
        if is_fence_line(line):
            in_fenced_code = not in_fenced_code
            source_lines.append(line)
            continue
        if not in_fenced_code and line.strip() == "!toc":
            continue
        source_lines.append(line)
    source = "\n".join(source_lines)
    headings = collect_headings(source)
    if not headings:
        return "## 目次\n\n- （項目なし）\n"

    base_level = min(level for level, _ in headings)
    lines = ["## 目次", ""]
    for level, title in headings:
        indent = "  " * max(0, level - base_level)
        lines.append(f"{indent}- {title}")
    return "\n".join(lines).rstrip() + "\n"


def expand_toc(markdown_text: str) -> str:
    if not has_directive_outside_fenced_code(markdown_text, predicate=lambda line: line.strip() == "!toc"):
        return markdown_text

    toc = build_toc(markdown_text)
    output_lines: list[str] = []
    in_fenced_code = False
    replaced = False
    for line in markdown_text.splitlines():
        if is_fence_line(line):
            in_fenced_code = not in_fenced_code
            output_lines.append(line)
            continue
        if not in_fenced_code and line.strip() == "!toc":
            output_lines.append(toc.rstrip())
            replaced = True
            continue
        output_lines.append(line)
    return "\n".join(output_lines) + ("\n" if replaced else "")


def normalize_page_break_spacing(markdown_text: str) -> str:
    if not markdown_text.strip():
        return ""

    output_lines: list[str] = []
    for line in markdown_text.splitlines():
        if line.strip() != PAGE_BREAK_MARKER:
            output_lines.append(line)
            continue

        while output_lines and not output_lines[-1].strip():
            output_lines.pop()
        if output_lines:
            output_lines.append("")
        output_lines.append(PAGE_BREAK_MARKER)
        output_lines.append("")

    normalized = "\n".join(output_lines).strip()
    return normalized + "\n" if normalized else ""


def join_pages(pages: list[str]) -> str:
    cleaned_pages = [page.strip() for page in pages if page.strip()]
    if not cleaned_pages:
        return ""
    return normalize_page_break_spacing(f"\n{PAGE_BREAK_MARKER}\n".join(cleaned_pages))


def expand_fit_code(markdown_text: str) -> str:
    lines = markdown_text.splitlines()
    output: list[str] = []
    in_fenced_code = False
    index = 0

    while index < len(lines):
        line = lines[index]
        if is_fence_line(line):
            in_fenced_code = not in_fenced_code
            output.append(line)
            index += 1
            continue

        if not in_fenced_code and line.strip() == "!fit-code":
            next_index = index + 1
            if next_index < len(lines) and is_fence_line(lines[next_index]):
                fence_line = lines[next_index]
                code_lines = [fence_line]
                next_index += 1
                while next_index < len(lines):
                    code_lines.append(lines[next_index])
                    if is_fence_line(lines[next_index]):
                        break
                    next_index += 1
                output.append('<div class="fit-code">')
                output.append("")
                output.extend(code_lines)
                output.append("")
                output.append("</div>")
                index = next_index + 1
                continue

        output.append(line)
        index += 1

    return "\n".join(output) + "\n"


def merge_pdfs(inputs: list[Path], output: Path) -> None:
    try:
        pypdf = importlib.import_module("pypdf")
    except ModuleNotFoundError as error:
        raise RuntimeError(
            "Merging page-specific footers requires the `pypdf` package. Install it with `python -m pip install pypdf`."
        ) from error

    writer = pypdf.PdfWriter()
    for pdf_path in inputs:
        reader = pypdf.PdfReader(str(pdf_path))
        for page in reader.pages:
            writer.add_page(page)

    with output.open("wb") as stream:
        writer.write(stream)


def split_pages(markdown_text: str) -> list[str]:
    normalized = normalize_page_break_spacing(markdown_text).strip()
    if not normalized:
        return []
    return [page.strip() + "\n" for page in normalized.split(PAGE_BREAK_MARKER)]


def _is_top_level_list_item(line: str) -> bool:
    return bool(re.match(r"^(?P<indent>[ \t]*)(?:[-+*]|\d+[.)])[ \t]+", line))


def _list_item_indent(line: str) -> int:
    match = re.match(r"^(?P<indent>[ \t]*)(?:[-+*]|\d+[.)])[ \t]+", line)
    if not match:
        return -1
    indent = match.group("indent")
    return len(indent.replace("\t", "    "))


def _extract_list_block(lines: list[str], start_index: int) -> tuple[list[list[str]], int]:
    items: list[list[str]] = []
    base_indent: int | None = None
    index = start_index

    while index < len(lines):
        line = lines[index]
        if not line.strip():
            if items:
                current = items[-1]
                next_index = index + 1
                if next_index < len(lines):
                    next_line = lines[next_index]
                    if next_line.strip() and not _is_top_level_list_item(next_line):
                        current.append(line)
                        index += 1
                        continue
            break

        if _is_top_level_list_item(line):
            indent = _list_item_indent(line)
            if base_indent is None:
                base_indent = indent
            elif indent != base_indent:
                if items:
                    items[-1].append(line)
                    index += 1
                    continue
                break

            items.append([line])
            index += 1
            continue

        if not items:
            break

        items[-1].append(line)
        index += 1

    return items, index


def _render_list_item_text(item: list[str]) -> str:
    first = re.sub(r"^(?P<indent>[ \t]*)(?:[-+*]|\d+[.)])[ \t]+", "", item[0]).strip()
    rest = [line.strip() for line in item[1:] if line.strip()]
    parts = [first, *rest]
    return "<br>".join(html.escape(part) for part in parts if part)


def render_incremental_list_html(items: list[list[str]], visible_count: int) -> str:
    first_line = items[0][0]
    ordered = bool(re.match(r"^[ \t]*\d+[.)][ \t]+", first_line))
    tag = "ol" if ordered else "ul"
    rendered_items: list[str] = []

    for index, item in enumerate(items):
        hidden_class = " progressive-hidden" if index >= visible_count else ""
        rendered_items.append(
            f'  <li class="{hidden_class.strip()}">{_render_list_item_text(item)}</li>'
            if hidden_class
            else f"  <li>{_render_list_item_text(item)}</li>"
        )

    return "\n".join([f'<{tag} class="incremental-list">', *rendered_items, f"</{tag}>"])


def expand_incremental_lists(markdown_text: str) -> str:
    expanded_pages: list[str] = []

    for page in split_pages(markdown_text):
        if not has_directive_outside_fenced_code(page, predicate=lambda line: line.strip() == "!incremental-list"):
            expanded_pages.append(page.strip() + "\n")
            continue

        lines = page.splitlines()
        directive_index = None
        in_fenced_code = False
        match_count = 0
        for index, line in enumerate(lines):
            if is_fence_line(line):
                in_fenced_code = not in_fenced_code
                continue
            if not in_fenced_code and line.strip() == "!incremental-list":
                match_count += 1
                directive_index = index
        if match_count > 1:
            raise ValueError("Only one `!incremental-list` directive is supported per page.")
        if directive_index is None:
            expanded_pages.append(page.strip() + "\n")
            continue

        list_start = directive_index + 1
        while list_start < len(lines) and not lines[list_start].strip():
            list_start += 1

        items, list_end = _extract_list_block(lines, list_start)
        if not items:
            raise ValueError("`!incremental-list` must be followed by a Markdown list.")

        prefix_lines = lines[:directive_index]
        suffix_lines = lines[list_end:]

        for count in range(1, len(items) + 1):
            page_lines = [*prefix_lines]
            if page_lines and page_lines[-1].strip():
                page_lines.append("")
            page_lines.append(render_incremental_list_html(items, count))
            if suffix_lines:
                if page_lines and page_lines[-1].strip() and suffix_lines[0].strip():
                    page_lines.append("")
                page_lines.extend(suffix_lines)
            expanded_pages.append("\n".join(page_lines).strip() + "\n")

    return join_pages(expanded_pages)


def expand_pause_directives(markdown_text: str) -> str:
    expanded_pages: list[str] = []

    for page in split_pages(markdown_text):
        footer_text, cleaned_page = extract_footer_text(page)
        lines = cleaned_page.splitlines()
        visible_lines: list[str] = []
        snapshots: list[str] = []
        pause_count = 0
        in_fenced_code = False

        for line in lines:
            if is_fence_line(line):
                in_fenced_code = not in_fenced_code
                visible_lines.append(line)
                continue

            if not in_fenced_code and line.strip() == "!pause":
                pause_count += 1
                snapshot_lines = [*visible_lines]
                if footer_text:
                    if snapshot_lines and snapshot_lines[-1].strip():
                        snapshot_lines.append("")
                    snapshot_lines.append(f"!footer{{{footer_text}}}")
                snapshots.append("\n".join(snapshot_lines).strip() + "\n")
                continue

            visible_lines.append(line)

        if pause_count == 0:
            expanded_pages.append(page.strip() + "\n")
            continue

        final_lines = [*visible_lines]
        if footer_text:
            if final_lines and final_lines[-1].strip():
                final_lines.append("")
            final_lines.append(f"!footer{{{footer_text}}}")
        snapshots.append("\n".join(final_lines).strip() + "\n")
        expanded_pages.extend(snapshots)

    return join_pages(expanded_pages)


def render_with_md_to_pdf(
    template_dir: Path,
    source: Path,
    markdown_text: str,
    output: Path,
    pdf_basedir: Path,
    footer_text: str | None = None,
    body_font_size: str | None = None,
    page_numbers: bool = False,
    page_number_override: int | None = None,
    total_pages_override: int | None = None,
    extra_env: dict[str, str] | None = None,
) -> int:
    if extra_env:
        markdown_text = apply_background_footer_margin(
            markdown_text,
            extra_env.get("PDF_BACKGROUND_FOOTER_MARGIN"),
        )

    with tempfile.NamedTemporaryFile(
        mode="w",
        encoding="utf-8",
        suffix=".md",
        prefix=f".{source.stem}.",
        delete=False,
        dir=source.parent,
    ) as temp_file:
        temp_path = Path(temp_file.name)
        temp_file.write(markdown_text)
        if not markdown_text.endswith("\n"):
            temp_file.write("\n")

    env = os.environ.copy()
    env["SOURCE_MARKDOWN_DIR"] = str(source.parent)
    env["PDF_BASEDIR"] = str(pdf_basedir)
    env["PDF_DEST"] = str(output)
    env.setdefault("NODE_NO_WARNINGS", "1")
    if footer_text is not None:
        env["PDF_FOOTER_TEXT"] = footer_text
    if font_size_css := build_body_font_size_style(body_font_size):
        env["PDF_BODY_FONT_SIZE"] = font_size_css
    if page_numbers:
        env["PDF_PAGE_NUMBERS"] = "1"
    if page_number_override is not None:
        env["PDF_PAGE_NUMBER_OVERRIDE"] = str(page_number_override)
    if total_pages_override is not None:
        env["PDF_TOTAL_PAGES_OVERRIDE"] = str(total_pages_override)
    if extra_env:
        env.update(extra_env)

    command = [
        *resolve_md_to_pdf_command(template_dir),
        "--config-file",
        str((template_dir / "config" / "md-to-pdf.config.js").resolve()),
        str(temp_path),
    ]

    try:
        completed = subprocess.run(command, env=env, check=False)
        return completed.returncode
    finally:
        if temp_path.exists():
            temp_path.unlink()


def render_with_manual_footers(
    template_dir: Path,
    source: Path,
    output: Path,
    pages: list[str],
    effective_format: str,
    title: str,
    subtitle: str,
    author: str,
    version: str,
    date_text: str,
    page_numbers: bool,
    global_font_size: str | None = None,
    html_prelude: str = "",
    extra_env: dict[str, str] | None = None,
) -> int:
    temp_dir = Path(tempfile.mkdtemp(prefix=f"{source.stem}-pages-"))
    page_pdfs: list[Path] = []

    try:
        for index, page in enumerate(pages):
            footer_text, cleaned_page = extract_footer_text(page)
            page_font_size, cleaned_page = extract_page_font_size(cleaned_page)
            prefix = build_prefix(
                template_dir,
                FORMATS[effective_format],
                title,
                subtitle,
                author,
                version,
                date_text,
                font_size=page_font_size or global_font_size,
                include_cover=index == 0,
                html_prelude=html_prelude,
            )
            page_markdown = prefix + cleaned_page
            page_output = temp_dir / f"page-{index:04d}.pdf"
            page_pdfs.append(page_output)
            pdf_basedir = determine_pdf_basedir(source, cleaned_page)
            status = render_with_md_to_pdf(
                template_dir,
                source,
                page_markdown,
                page_output,
                pdf_basedir,
                footer_text=footer_text,
                body_font_size=page_font_size or global_font_size,
                page_numbers=page_numbers,
                page_number_override=index + 1 if page_numbers else None,
                total_pages_override=len(pages) if page_numbers else None,
                extra_env=extra_env,
            )
            if status != 0:
                return status

        merge_pdfs(page_pdfs, output)
        return 0
    finally:
        for path in page_pdfs:
            if path.exists():
                path.unlink()
        if temp_dir.exists():
            temp_dir.rmdir()


def render_markdown(args: argparse.Namespace) -> int:
    source = normalize_platform_path(args.source).expanduser().resolve()
    if not source.is_file():
        raise FileNotFoundError(f"Markdown file not found: {source}")

    resource_dir = os.environ.get("GENPDF_RESOURCE_DIR")
    if resource_dir:
        template_dir = normalize_platform_path(resource_dir).expanduser().resolve()
    else:
        template_dir = normalize_platform_path(__file__).resolve().parent / "resources"
    source_text = source.read_text(encoding="utf-8-sig")
    embedded_settings, source_without_settings = parse_leading_front_matter(source_text)
    effective_format = args.format or embedded_settings.get("format") or "book"
    title = args.title if args.title is not None else embedded_settings.get("title")
    subtitle = args.subtitle if args.subtitle is not None else embedded_settings.get("subtitle") or ""
    author = args.author if args.author is not None else embedded_settings.get("author") or ""
    version = embedded_settings.get("version") or ""
    copyright_text = normalize_copyright_text(
        args.copyright if args.copyright is not None else embedded_settings.get("copyright")
    )
    page_numbers = (
        args.page_numbers
        if args.page_numbers is not None
        else parse_bool_setting(embedded_settings.get("page_numbers"))
    )
    footer_page_numbers = page_numbers and not has_bottom_background(embedded_settings)
    font_size = args.font_size if args.font_size is not None else embedded_settings.get("font_size")
    if args.date is not None:
        date_text = args.date
    elif "date" in embedded_settings:
        date_text = embedded_settings["date"]
    else:
        date_text = date.today().isoformat()
    body = source_text if args.passthrough else source_without_settings.lstrip()
    if effective_format == "book" and has_directive_outside_fenced_code(body, predicate=lambda line: line.strip() == "!toc"):
        body = expand_toc(body)
    if has_directive_outside_fenced_code(body, predicate=lambda line: line.strip() == "!fit-code"):
        body = expand_fit_code(body)
    if effective_format == "slides" and has_directive_outside_fenced_code(body, predicate=lambda line: line.strip() == "!incremental-list"):
        body = expand_incremental_lists(body)
    if effective_format == "slides" and has_directive_outside_fenced_code(body, predicate=lambda line: line.strip() == "!pause"):
        body = expand_pause_directives(body)
    body = normalize_page_break_spacing(body)
    uses_dot = has_dot_code_block(body)
    title = title or detect_title(body, source.stem)
    uses_mermaid = has_mermaid_code_block(body)
    mermaid_bundle_path = resolve_mermaid_bundle_path(template_dir) if uses_mermaid else None
    mermaid_html_prelude = build_mermaid_html_prelude(mermaid_bundle_path) if uses_mermaid else ""
    background_html_prelude = build_background_html_prelude(embedded_settings, source.parent, effective_format)
    background_footer_env = build_background_footer_env(embedded_settings, source.parent)
    table_html_prelude = build_table_html_prelude(embedded_settings)
    html_prelude = background_html_prelude + table_html_prelude + mermaid_html_prelude
    output = Path(args.output).expanduser().resolve() if args.output else source.with_suffix(".pdf")
    page_notes, body = extract_notes(body)
    notes_output = output.with_suffix(".notes.md")
    write_notes_file(notes_output, page_notes)
    validate_runtime_requirements(
        template_dir,
        requires_pdf_merge=has_directive_outside_fenced_code(
            body,
            predicate=lambda line: re.match(r"^!footer\{[^}]*\}[ \t]*$", line) is not None,
        ),
        requires_dot=uses_dot,
    )
    if uses_dot:
        body = expand_dot_diagrams(body)
    if has_directive_outside_fenced_code(
        body,
        predicate=lambda line: re.match(r"^!footer\{[^}]*\}[ \t]*$", line) is not None,
    ):
        if copyright_text:
            raise ValueError("`!footer{...}` and copyright footer cannot be used together.")
        pages = split_pages(body)
        status = render_with_manual_footers(
            template_dir,
            source,
            output,
            pages,
            effective_format,
            title,
            subtitle,
            author,
            version,
            date_text,
            footer_page_numbers,
            global_font_size=font_size,
            html_prelude=html_prelude,
            extra_env=background_footer_env,
        )
        if status == 0:
            apply_bottom_background_to_pdf(output, embedded_settings, source.parent, effective_format, page_numbers)
        return status

    if has_directive_outside_fenced_code(
        body,
        predicate=lambda line: FONT_SIZE_DIRECTIVE_PATTERN.match(line) is not None,
    ):
        pages = split_pages(body)
        status = render_with_manual_footers(
            template_dir,
            source,
            output,
            pages,
            effective_format,
            title,
            subtitle,
            author,
            version,
            date_text,
            footer_page_numbers,
            global_font_size=font_size,
            html_prelude=html_prelude,
            extra_env=background_footer_env,
        )
        if status == 0:
            apply_bottom_background_to_pdf(output, embedded_settings, source.parent, effective_format, page_numbers)
        return status

    prefix = "" if args.passthrough else build_prefix(
        template_dir,
        FORMATS[effective_format],
        title,
        subtitle,
        author,
        version,
        date_text,
        font_size=font_size,
        html_prelude=html_prelude,
    )
    pdf_basedir = determine_pdf_basedir(source, body)
    status = render_with_md_to_pdf(
        template_dir,
        source,
        prefix + body,
        output,
        pdf_basedir,
        footer_text=copyright_text,
        body_font_size=font_size,
        page_numbers=footer_page_numbers,
        extra_env=background_footer_env,
    )
    if status == 0:
        apply_bottom_background_to_pdf(output, embedded_settings, source.parent, effective_format, page_numbers)
    return status


def main() -> None:
    args = parse_args()
    warn_deprecated_cli_options(args)
    if args.front_matter_template:
        print(build_front_matter_template(), end="")
        return
    try:
        raise SystemExit(render_markdown(args))
    except (FileNotFoundError, RuntimeError, ValueError) as error:
        print(error, file=sys.stderr)
        raise SystemExit(2) from error


if __name__ == "__main__":
    main()
