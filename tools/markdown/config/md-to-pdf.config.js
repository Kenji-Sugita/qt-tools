// md-to-pdf.config.js
const fs = require("fs");
const path = require("path");

function escapeHtml(value) {
  return String(value)
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;")
    .replace(/'/g, "&#39;");
}

function dataUriForFile(fullPath) {
  const extension = path.extname(fullPath).toLowerCase();
  const mimeTypes = {
    ".svg": "image/svg+xml",
    ".png": "image/png",
    ".jpg": "image/jpeg",
    ".jpeg": "image/jpeg",
    ".gif": "image/gif",
    ".webp": "image/webp"
  };
  const mimeType = mimeTypes[extension] || "application/octet-stream";
  const bytes = fs.readFileSync(fullPath);
  return `data:${mimeType};base64,${bytes.toString("base64")}`;
}

// 使用例: `!include-code{path=src/main.py lang=python}`
// 行範囲を絞るなら `lines=10-30` のように指定
function includeCodeExtension(baseDir = process.env.SOURCE_MARKDOWN_DIR || process.cwd()) {
  return {
    extensions: [{
      name: "include-code",
      level: "block",
      start(src) {
        const m = src.match(/^!include-code\{/m);
        return m ? m.index : undefined;
      },
      tokenizer(src) {
        const rule = /^!include-code\{([^}]+)\}[ \t]*\n?/;
        const m = rule.exec(src);
        if (!m) return;

        // 引数を "key=value" の空白区切りで解釈
        const args = Object.fromEntries(
          m[1].split(/\s+/).map(kv => {
            const [k, ...rest] = kv.split("=");
            return [k, rest.join("=")];
          })
        );

        const file = args.path || args.file;
        if (!file) return;

        const full = path.resolve(baseDir, file);
        let text = fs.readFileSync(full, "utf8");

        // 行範囲の抽出: lines=10-30, 10-, -30 も可
        if (args.lines) {
          const [sRaw, eRaw] = String(args.lines).split("-");
          const s = sRaw ? parseInt(sRaw, 10) : 1;
          const e = eRaw ? parseInt(eRaw, 10) : Infinity;
          const lines = text.split(/\r?\n/);
          text = lines.slice(Math.max(0, s - 1), Math.min(e, lines.length)).join("\n");
        }

        return {
          type: "code",
          raw: m[0],
          text,
          lang: args.lang || ""
        };
      }
    }]
  };
}

function svgFigureExtension(baseDir = process.env.SOURCE_MARKDOWN_DIR || process.cwd()) {
  return {
    extensions: [{
      name: "svg-figure",
      level: "block",
      start(src) {
        const m = src.match(/^!svg\{/m);
        return m ? m.index : undefined;
      },
      tokenizer(src) {
        const rule = /^!svg\{([^}]+)\}[ \t]*\n?/;
        const m = rule.exec(src);
        if (!m) return;

        const args = Object.fromEntries(
          m[1].split(/\s+/).map(kv => {
            const [k, ...rest] = kv.split("=");
            return [k, rest.join("=")];
          })
        );

        const file = args.path || args.file || args.src;
        if (!file) return;

        const full = path.resolve(baseDir, file);
        if (!fs.existsSync(full)) {
          throw new Error(`svg file not found: ${full}`);
        }

        const rawWidth = args.width ? String(args.width).trim() : "100%";
        const width = /^[0-9.]+(%|px|vw|vh|em|rem)?$/.test(rawWidth) ? rawWidth : "100%";
        const requestedAlign = String(args.align || "center").trim().toLowerCase();
        const align = ["left", "center", "right"].includes(requestedAlign) ? requestedAlign : "center";
        const imageSrc = dataUriForFile(full);
        const alt = escapeHtml(args.alt || "");
        const caption = args.caption ? `<figcaption>${escapeHtml(args.caption)}</figcaption>` : "";

        return {
          type: "html",
          raw: m[0],
          text: `
<figure class="svg-figure svg-align-${escapeHtml(align)}">
  <img src="${imageSrc}" alt="${alt}" style="width:${escapeHtml(width)};">
  ${caption}
</figure>`
        };
      }
    }]
  };
}

function mermaidExtension() {
  return {
    extensions: [{
      name: "mermaid",
      level: "block",
      start(src) {
        const m = src.match(/^(?:!mermaid\{[^}]*\}$|(`{3,}|~{3,})[ \t]*mermaid(?:[ \t]+[^\n]*)?$)/m);
        return m ? m.index : undefined;
      },
      tokenizer(src) {
        const rule = /^(?:!mermaid\{([^}]*)\}[ \t]*\n)?(`{3,}|~{3,})[ \t]*mermaid(?:[ \t]+[^\n]*)?\n([\s\S]*?)\n\2[ \t]*(?:\n|$)/;
        const m = rule.exec(src);
        if (!m) return;

        const args = Object.fromEntries(
          String(m[1] || "")
            .split(/\s+/)
            .filter(Boolean)
            .map(kv => {
              const [k, ...rest] = kv.split("=");
              return [k, rest.join("=")];
            })
        );
        const requestedAlign = String(args.align || "center").trim().toLowerCase();
        const align = ["left", "center", "right"].includes(requestedAlign) ? requestedAlign : "center";

        return {
          type: "mermaid",
          raw: m[0],
          text: m[3].trim(),
          align
        };
      },
      renderer(token) {
        return `<div class="mermaid mermaid-align-${escapeHtml(token.align)}">${escapeHtml(token.text)}</div>`;
      }
    }]
  };
}

const footerText = (process.env.PDF_FOOTER_TEXT || "").trim();
const bodyFontSize = (process.env.PDF_BODY_FONT_SIZE || "").trim();
const pageNumbers = ["1", "true", "yes", "on"].includes((process.env.PDF_PAGE_NUMBERS || "").trim().toLowerCase());
const pageNumberOverride = (process.env.PDF_PAGE_NUMBER_OVERRIDE || "").trim();
const totalPagesOverride = (process.env.PDF_TOTAL_PAGES_OVERRIDE || "").trim();
const pageNumberHtml = pageNumbers
  ? (
      pageNumberOverride
        ? `${escapeHtml(pageNumberOverride)}${totalPagesOverride ? ` / ${escapeHtml(totalPagesOverride)}` : ""}`
        : '<span class="pageNumber"></span> / <span class="totalPages"></span>'
    )
  : "";
const footerTemplate = (footerText || pageNumbers)
  ? `
    <style>
      .footer {
        width: 100%;
        font-family: Geneva, Tahoma, Verdana, "Helvetica Neue", Helvetica, Arial, sans-serif;
        font-size: 10px;
        color: #444;
        padding: 0 12mm;
        display: flex;
        align-items: center;
        justify-content: space-between;
      }

      .footer-left {
        text-align: left;
      }

      .footer-right {
        text-align: right;
      }
    </style>
    <section class="footer">
      <div class="footer-left">${escapeHtml(footerText)}</div>
      <div class="footer-right">${pageNumberHtml}</div>
    </section>
  `
  : undefined;

module.exports = {
  basedir: process.env.PDF_BASEDIR || process.cwd(),
  dest: process.env.PDF_DEST,
  css: bodyFontSize
    ? `
      p,
      li,
      h2,
      h3,
      h4,
      h5,
      h6,
      blockquote,
      blockquote p,
      blockquote li,
      table th,
      table td,
      pre,
      pre code,
      figure.cover-image figcaption,
      .callout-body,
      .callout-body p,
      .callout-body li {
        font-size: ${bodyFontSize} !important;
      }
    `
    : undefined,
  pdf_options: footerTemplate
    ? {
        displayHeaderFooter: true,
        footerTemplate
      }
    : {},
  // Marked に拡張を渡します
  marked_extensions: [includeCodeExtension(), svgFigureExtension(), mermaidExtension()],
  // 必要なら Marked のオプションもここで
  // marked_options: { gfm: true }
};
