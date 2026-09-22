#!/usr/bin/env python3
"""Build the onboarding course as HTML pages in onboarding/site/.

The Markdown files in onboarding/ are the source. After editing any of them,
regenerate the pages from the repo root:

    python3 -m pip install markdown pymdown-extensions   # once
    python3 onboarding/tools/build_site.py

The generated pages need nothing installed to view: open
onboarding/site/index.html in a browser. Diagram pages load the Mermaid
library from the internet and show the diagram source when offline.
"""

import html
import os
import re
import sys

try:
    import markdown
    import pymdownx.superfences
except ImportError:
    sys.exit("Missing libraries. Run:  python3 -m pip install markdown pymdown-extensions")

HERE = os.path.dirname(os.path.abspath(__file__))
ONB = os.path.dirname(HERE)
ROOT = os.path.dirname(ONB)
SITE = os.path.join(ONB, "site")
GITHUB = "https://github.com/rome8721/Ahmed_Hira"

DIAGRAMS = ["architecture", "request-flow", "data-model", "computer-turn",
            "new-round", "round-end", "save-game", "resume-game"]

# (markdown path relative to onboarding/, output page, short nav label)
PAGES = [("ONBOARDING.md", "onboarding.html", "Guide"),
         ("cpp-primer.md", "cpp-primer.html", "C++ primer")]
PAGES += [(f"days/day-{n}.md", f"day-{n}.html", f"Day {n}") for n in range(1, 7)]
PAGES += [(f"diagrams/{d}.md", f"diagram-{d}.html", d) for d in DIAGRAMS]
MD_TO_PAGE = {md: page for md, page, _ in PAGES}


def slugify(value, separator="-"):
    """GitHub-style heading ids, so existing #anchors keep working."""
    value = re.sub(r"[^\w\- ]", "", value.strip().lower())
    return value.replace(" ", separator)


def preprocess(text):
    """The sources indent nested list content by 3 spaces; Python-Markdown needs 4."""
    out, in_root_fence = [], False
    for line in text.split("\n"):
        if line.startswith("```"):
            in_root_fence = not in_root_fence
        elif not in_root_fence:
            if re.match(r"^ {3}(?! )", line):
                line = " " + line
            # GitHub starts a list or table right after a paragraph line;
            # Python-Markdown needs a blank line first.
            prev = out[-1] if out else ""
            starts_list = re.match(r"^(\d+\.|[-*]) ", line) and prev.strip() \
                and not re.match(r"^(\d+\.|[-*]) |^\s", prev)
            starts_table = line.startswith("|") and prev.strip() and not prev.startswith("|")
            if starts_list or starts_table:
                out.append("")
        out.append(line)
    return "\n".join(out)


def convert(text):
    md = markdown.Markdown(
        extensions=["tables", "toc", "md_in_html", "pymdownx.superfences", "pymdownx.tasklist"],
        extension_configs={
            "toc": {"slugify": slugify},
            "pymdownx.superfences": {"custom_fences": [
                {"name": "mermaid", "class": "mermaid",
                 "format": pymdownx.superfences.fence_div_format}]},
        })
    return md.convert(preprocess(text))


def map_href(href, md_dir):
    """Turn a link written for the Markdown files into one that works from site/."""
    if href.startswith(("http://", "https://", "mailto:", "#", "vscode:")):
        return href, None
    path, _, anchor = href.partition("#")
    target = os.path.normpath(os.path.join(ONB, md_dir, path))
    rel = os.path.relpath(target, ROOT).replace(os.sep, "/")
    suffix = "#" + anchor if anchor else ""
    if rel.startswith("onboarding/") or rel == "onboarding":
        sub = rel[len("onboarding/"):]
        if sub in MD_TO_PAGE:
            return MD_TO_PAGE[sub] + suffix, None
        if sub == "diagrams":
            return "index.html#diagrams", None
        if sub == "days":
            return "index.html#days", None
        link = os.path.relpath(target, SITE).replace(os.sep, "/")
        return link + ("/" if path.endswith("/") else "") + suffix, None
    if rel.endswith(".pdf"):
        return os.path.relpath(target, SITE).replace(os.sep, "/"), None
    if os.path.isdir(target):
        return f"{GITHUB}/tree/main/{rel}", None
    line = re.match(r"L(\d+)", anchor)
    return f"{GITHUB}/blob/main/{rel}{suffix}", (rel, line.group(1) if line else "")


def page_titles():
    titles = {}
    for md_rel, out, label in PAGES:
        m = re.search(r"^# (.+)$", open(os.path.join(ONB, md_rel), encoding="utf-8").read(), re.M)
        titles[out] = html.escape(m.group(1).strip()) if m else label
    return titles


def friendly_link_text(body, titles):
    """Links written as file names (like diagrams/request-flow.md) show the page title instead."""
    def repl(m):
        page = m.group(1).split("#")[0]
        text = re.sub(r"<[^>]+>", "", m.group(3))
        if page in titles and text.strip().endswith(".md"):
            return f'<a href="{m.group(1)}"{m.group(2)}>{titles[page]}</a>'
        return m.group(0)
    return re.sub(r'<a href="([^"]+)"([^>]*)>(.*?)</a>', repl, body)


def rewrite_links(body, md_dir):
    def repl(m):
        href, code = map_href(html.unescape(m.group(1)), md_dir)
        attrs = f'href="{html.escape(href)}"'
        if code:
            attrs += f' data-code="{code[0]}" data-line="{code[1]}" class="code-link"'
        return "<a " + attrs
    return re.sub(r'<a href="([^"]*)"', repl, body)


def enhance_day(body):
    # A "done" checkbox on every exercise heading.
    m = re.search(r'(<h2 id="hands-on-exercises">.*?</h2>)(.*?)(?=<h2 )', body, re.S)
    if m:
        section = re.sub(r"(<h3[^>]*>)(.*?)(</h3>)",
                         r'\1<label class="ex-done"><input type="checkbox" disabled> \2</label>\3', m.group(2))
        body = body[:m.start(2)] + section + body[m.end(2):]
    # Day 5: each bug ticket. Day 6: preparing and giving the explanation.
    body = re.sub(r'(<h[23] id="(?:bug-ticket|after-all-three|prepare-\d|present-\d)[^"]*">)(.*?)(</h[23]>)',
                  r'\1<label class="ex-done"><input type="checkbox" disabled> \2</label>\3', body)
    # Day 6: each acceptance criterion.
    m = re.search(r'(<h3 id="acceptance-criteria">.*?<ol>)(.*?)(</ol>)', body, re.S)
    if m:
        nested = []
        items = re.sub(r"<ul>.*?</ul>", lambda x: nested.append(x.group(0)) or f"\0{len(nested) - 1}\0",
                       m.group(2), flags=re.S)
        items = items.replace("<li>", '<li><input type="checkbox" disabled> ')
        items = re.sub(r"\0(\d+)\0", lambda x: nested[int(x.group(1))], items)
        body = body[:m.start(2)] + items + body[m.end(2):]
    # Interactive checkboxes (state is kept in the browser).
    counter = iter(range(1000))
    body = re.sub(r'<input type="checkbox" disabled(?:="disabled")?\s*/?>',
                  lambda m: f'<input type="checkbox" class="task" data-i="{next(counter)}">', body)
    # An answer box under each check question.
    m = re.search(r'(<h2 id="check-questions">.*?</h2>\s*<ol>)(.*?)(</ol>)', body, re.S)
    if m:
        qcount = iter(range(100))
        items = re.sub(r"</li>", lambda _: (
            f'<textarea class="answer" data-q="{next(qcount)}" rows="3" '
            'placeholder="Write your answer here before opening the answer key"></textarea></li>'), m.group(2))
        body = body[:m.start()] + m.group(1) + items + m.group(3) + body[m.end():]
    # Hide the answer key until the learner asks for it.
    m = re.search(r'<h2 id="answer-key">.*?</h2>', body)
    if m:
        body = (body[:m.start()] + '<details class="answer-key"><summary>Show the answer key '
                '<span>(answer the questions first!)</span></summary>' + body[m.end():] + "</details>")
    return body, sum(1 for _ in re.finditer(r'class="task"', body))


def extract(md_text, label):
    m = re.search(r"^\*\*" + label + r":\*\*\s*(.+)$", md_text, re.M)
    return markdown.markdown(m.group(1)).replace("<p>", "").replace("</p>", "") if m else ""


CSS = r"""
:root {
  --bg: #f7f7f5; --panel: #ffffff; --panel-2: #f0f1f3; --text: #1d232e; --muted: #5c6575;
  --border: #dde1e7; --accent: #2f6fdd; --accent-soft: #e4ecfb; --ok: #1e8a4c; --ok-soft: #e2f4e9;
  --warn-soft: #fbf1dc; --code-bg: #f3f4f6; --shadow: 0 1px 2px rgba(20,30,50,.05), 0 2px 10px rgba(20,30,50,.05);
  --mono: ui-monospace, "Cascadia Code", Consolas, Menlo, monospace;
  --sans: "Segoe UI", -apple-system, BlinkMacSystemFont, Roboto, Helvetica, Arial, sans-serif;
}
@media (prefers-color-scheme: dark) {
  :root:not([data-theme="light"]) {
    --bg: #0f1218; --panel: #171b23; --panel-2: #1f2430; --text: #e5e8ee; --muted: #9aa3b3;
    --border: #2c3341; --accent: #6d9ef0; --accent-soft: #1c2a44; --ok: #5fcf8c; --ok-soft: #173325;
    --warn-soft: #33290f; --code-bg: #1c212b; --shadow: none;
  }
}
:root[data-theme="dark"] {
  --bg: #0f1218; --panel: #171b23; --panel-2: #1f2430; --text: #e5e8ee; --muted: #9aa3b3;
  --border: #2c3341; --accent: #6d9ef0; --accent-soft: #1c2a44; --ok: #5fcf8c; --ok-soft: #173325;
  --warn-soft: #33290f; --code-bg: #1c212b; --shadow: none;
}
* { box-sizing: border-box; }
html, body { margin: 0; }
body { background: var(--bg); color: var(--text); font: 16px/1.6 var(--sans); }
a { color: var(--accent); }
a.code-link::after { content: " ↗"; font-size: .8em; }
.topbar { position: sticky; top: 0; z-index: 5; background: var(--panel); border-bottom: 1px solid var(--border); }
.topbar-inner { max-width: 1100px; margin: 0 auto; padding: 8px 16px; display: flex; flex-wrap: wrap; align-items: center; gap: 6px 14px; }
.brand { font-weight: 700; text-decoration: none; color: var(--text); margin-right: 6px; }
.nav { display: flex; flex-wrap: wrap; gap: 4px; }
.nav a { text-decoration: none; color: var(--muted); font-size: 14px; padding: 3px 9px; border-radius: 7px; }
.nav a:hover { background: var(--panel-2); color: var(--text); }
.nav a.cur { background: var(--accent-soft); color: var(--accent); font-weight: 600; }
.nav a.done::after { content: " ✓"; color: var(--ok); }
.theme-btn { margin-left: auto; border: 1px solid var(--border); background: var(--panel); color: var(--muted); border-radius: 7px; padding: 3px 9px; font: inherit; font-size: 13px; cursor: pointer; }
main { max-width: 860px; margin: 0 auto; padding: 24px 16px 60px; }
main h1 { font-size: 30px; line-height: 1.25; margin: 8px 0 12px; letter-spacing: -.01em; }
main h2 { font-size: 22px; margin: 36px 0 10px; padding-top: 8px; border-top: 1px solid var(--border); }
main h3 { font-size: 18px; margin: 26px 0 8px; }
main hr { border: 0; border-top: 1px solid var(--border); margin: 28px 0; }
main hr + hr { display: none; }
code { font-family: var(--mono); font-size: .9em; background: var(--code-bg); padding: 1px 5px; border-radius: 4px; }
pre { position: relative; background: var(--code-bg); border: 1px solid var(--border); border-radius: 8px; padding: 12px 14px; overflow-x: auto; }
pre code { background: none; padding: 0; font-size: 13.5px; line-height: 1.5; }
.copy-btn { position: absolute; top: 6px; right: 6px; border: 1px solid var(--border); background: var(--panel); color: var(--muted); border-radius: 6px; font: 12px var(--sans); padding: 2px 8px; cursor: pointer; opacity: .8; }
.copy-btn:hover { opacity: 1; color: var(--text); }
.table-wrap { overflow-x: auto; margin: 14px 0; }
table { border-collapse: collapse; width: 100%; font-size: 14.5px; background: var(--panel); }
th, td { border: 1px solid var(--border); padding: 7px 10px; text-align: left; vertical-align: top; }
th { background: var(--panel-2); }
blockquote { margin: 16px 0; padding: 10px 16px; background: var(--warn-soft); border-left: 4px solid #d9a320; border-radius: 0 8px 8px 0; }
blockquote p { margin: 6px 0; }
details { background: var(--panel); border: 1px solid var(--border); border-radius: 8px; padding: 8px 12px; margin: 8px 0; }
details summary { cursor: pointer; font-weight: 600; }
details.answer-key { margin-top: 30px; border: 2px dashed var(--border); padding: 12px 16px; }
details.answer-key summary { font-size: 18px; }
details.answer-key summary span { font-weight: 400; color: var(--muted); font-size: 14px; }
li.task-list-item { list-style: none; margin-left: -1.3em; }
label.ex-done { cursor: pointer; }
input.task { width: 17px; height: 17px; margin-right: 8px; vertical-align: -3px; accent-color: var(--ok); cursor: pointer; }
textarea.answer { display: block; width: 100%; margin: 6px 0 12px; font: 14px/1.5 var(--sans); color: var(--text); background: var(--panel); border: 1px solid var(--border); border-radius: 8px; padding: 8px 10px; resize: vertical; }
.box { background: var(--panel); border: 1px solid var(--border); border-radius: 12px; padding: 14px 18px; margin: 18px 0; box-shadow: var(--shadow); }
.box h4 { margin: 0 0 6px; font-size: 15px; }
.box p { margin: 6px 0; }
.start-box { border-left: 4px solid var(--accent); }
.pager { display: flex; justify-content: space-between; gap: 12px; margin-top: 40px; flex-wrap: wrap; }
.pager a { text-decoration: none; border: 1px solid var(--border); background: var(--panel); border-radius: 10px; padding: 10px 14px; box-shadow: var(--shadow); }
.done-btn { font: inherit; font-weight: 600; border: 1px solid var(--ok); color: var(--ok); background: var(--ok-soft); border-radius: 10px; padding: 10px 16px; cursor: pointer; }
.done-btn.is-done { background: var(--ok); color: #fff; }
.progress { height: 8px; background: var(--panel-2); border-radius: 99px; overflow: hidden; margin: 8px 0 4px; }
.progress > div { height: 100%; background: var(--ok); width: 0; transition: width .3s; }
.muted { color: var(--muted); font-size: 14px; }
.cards { display: grid; grid-template-columns: repeat(auto-fill, minmax(250px, 1fr)); gap: 12px; margin: 14px 0; }
.card { display: block; text-decoration: none; color: var(--text); background: var(--panel); border: 1px solid var(--border); border-radius: 12px; padding: 14px 16px; box-shadow: var(--shadow); }
.card:hover { border-color: var(--accent); }
.card h3 { margin: 0 0 4px; font-size: 17px; }
.card .badge { font-size: 12px; font-weight: 700; color: var(--ok); }
.card p { margin: 6px 0; font-size: 14px; color: var(--muted); }
.mermaid { background: var(--panel); border: 1px solid var(--border); border-radius: 10px; padding: 12px; margin: 14px 0; overflow-x: auto; }
.mermaid.no-js { white-space: pre; font-family: var(--mono); font-size: 12.5px; }
.offline-note { display: none; }
.mermaid.no-js + .offline-note, .show-offline .offline-note { display: block; }
@media (max-width: 600px) { main h1 { font-size: 25px; } body { font-size: 15.5px; } }
"""

JS = r"""
(function () {
  var page = document.body.dataset.page;
  function get(k) { try { return localStorage.getItem(k); } catch (e) { return null; } }
  function set(k, v) { try { if (v === null) localStorage.removeItem(k); else localStorage.setItem(k, v); } catch (e) {} }

  // Theme toggle
  var saved = get("course:theme");
  if (saved === "light" || saved === "dark") document.documentElement.dataset.theme = saved;
  var tb = document.getElementById("themeBtn");
  if (tb) tb.addEventListener("click", function () {
    var r = document.documentElement;
    var dark = r.dataset.theme ? r.dataset.theme === "dark" : matchMedia("(prefers-color-scheme: dark)").matches;
    r.dataset.theme = dark ? "light" : "dark"; set("course:theme", r.dataset.theme);
  });

  // Code links open the file in VS Code when the page is opened from disk.
  var root = null;
  if (location.protocol === "file:") {
    root = decodeURIComponent(location.pathname).replace(/\/onboarding\/site\/[^\/]*$/, "").replace(/^\//, "");
  }
  document.querySelectorAll("a[data-code]").forEach(function (a) {
    if (!root) return;
    a.href = "vscode://file/" + root + "/" + a.dataset.code + (a.dataset.line ? ":" + a.dataset.line : "");
    a.title = "Open in VS Code" + (a.dataset.line ? " at line " + a.dataset.line : "");
  });

  // Copy buttons on code blocks
  document.querySelectorAll("pre > code").forEach(function (code) {
    var b = document.createElement("button");
    b.className = "copy-btn"; b.type = "button"; b.textContent = "Copy";
    b.addEventListener("click", function () {
      var text = code.innerText.replace(/\s+#[^\n]*$/gm, "").replace(/\n+$/, "");
      function done() { b.textContent = "Copied"; setTimeout(function () { b.textContent = "Copy"; }, 1200); }
      if (navigator.clipboard && window.isSecureContext) navigator.clipboard.writeText(text).then(done, fallback); else fallback();
      function fallback() {
        var t = document.createElement("textarea"); t.value = text; document.body.appendChild(t); t.select();
        try { document.execCommand("copy"); done(); } catch (e) {} document.body.removeChild(t);
      }
    });
    code.parentNode.appendChild(b);
  });

  // Checkboxes and answers are remembered in this browser.
  document.querySelectorAll("input.task").forEach(function (cb) {
    var k = "course:" + page + ":task:" + cb.dataset.i;
    cb.checked = get(k) === "1";
    cb.addEventListener("change", function () { set(k, cb.checked ? "1" : null); });
  });
  document.querySelectorAll("textarea.answer").forEach(function (ta) {
    var k = "course:" + page + ":q:" + ta.dataset.q;
    ta.value = get(k) || "";
    ta.addEventListener("input", function () { set(k, ta.value || null); });
  });

  // "Day complete" button
  var db = document.getElementById("doneBtn");
  function paintDone() {
    var d = get("course:" + page + ":done") === "1";
    if (!db) return;
    db.classList.toggle("is-done", d);
    db.textContent = d ? "✓ Day complete (click to undo)" : "Mark this day complete";
  }
  if (db) { db.addEventListener("click", function () {
    var k = "course:" + page + ":done"; set(k, get(k) === "1" ? null : "1"); paintDone(); markNav();
  }); paintDone(); }

  function markNav() {
    document.querySelectorAll(".nav a[data-day]").forEach(function (a) {
      a.classList.toggle("done", get("course:" + a.dataset.day + ":done") === "1");
    });
  }
  markNav();

  // Home page progress
  document.querySelectorAll("[data-progress]").forEach(function (el) {
    var p = el.dataset.progress, total = Number(el.dataset.total), n = 0;
    for (var i = 0; i < total; i++) if (get("course:" + p + ":task:" + i) === "1") n++;
    var done = get("course:" + p + ":done") === "1";
    var bar = el.querySelector(".progress > div"); if (bar) bar.style.width = (done ? 100 : total ? Math.round(100 * n / total) : 0) + "%";
    var label = el.querySelector(".plabel");
    if (label) label.textContent = done ? "Complete" : (total ? n + " of " + total + " checklist items done" : "Not started");
    var badge = el.querySelector(".badge"); if (badge) badge.textContent = done ? "✓ COMPLETE" : "";
  });

  // Diagrams: render with Mermaid if it loaded, otherwise show the source.
  var diagrams = document.querySelectorAll(".mermaid");
  if (diagrams.length) {
    if (window.mermaid) {
      var dark = document.documentElement.dataset.theme === "dark" ||
        (!document.documentElement.dataset.theme && matchMedia("(prefers-color-scheme: dark)").matches);
      mermaid.initialize({ startOnLoad: false, theme: dark ? "dark" : "default" });
      mermaid.run({ querySelector: ".mermaid" }).catch(function () { document.body.classList.add("show-offline"); });
    } else {
      diagrams.forEach(function (d) { d.classList.add("no-js"); });
    }
  }
})();
"""


def nav_html(current):
    links = [("index.html", "Home", "index")]
    links += [("onboarding.html", "Guide", "onboarding"), ("cpp-primer.html", "C++ primer", "cpp-primer")]
    links += [(f"day-{n}.html", f"Day {n}", f"day-{n}") for n in range(1, 7)]
    links += [("../simulator.html", "Simulator ↗", "simulator")]
    out = []
    for href, label, key in links:
        cls = ' class="cur"' if key == current else ""
        day = f' data-day="{key}"' if key.startswith("day-") else ""
        out.append(f'<a href="{href}"{cls}{day}>{label}</a>')
    return "".join(out)


def page_html(title, key, body, extra_head=""):
    return f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>{html.escape(title)}</title>
<style>{CSS}</style>
{extra_head}
</head>
<body data-page="{key}">
<header class="topbar"><div class="topbar-inner">
<a class="brand" href="index.html">35 Domino course</a>
<nav class="nav">{nav_html(key)}</nav>
<button class="theme-btn" id="themeBtn" type="button">Theme</button>
</div></header>
<main>
{body}
</main>
<script>{JS}</script>
</body>
</html>
"""


START_BOX = """<div class="box start-box">
<h4>Start of day</h4>
<p>Open VS Code, open the terminal with <code>Ctrl+`</code>, and get the latest version of the course:</p>
<pre><code>cd ~/Ahmed_Hira
git pull</code></pre>
<p class="muted">Links marked ↗ open the file in VS Code at the right line. The first time, your browser asks permission to open VS Code: allow it.
Checkboxes and your answers are saved in this browser.</p>
</div>"""


def build():
    global TITLES
    TITLES = page_titles()
    os.makedirs(SITE, exist_ok=True)
    day_info = []
    for md_rel, out, label in PAGES:
        src = open(os.path.join(ONB, md_rel), encoding="utf-8").read()
        md_dir = os.path.dirname(md_rel)
        body = friendly_link_text(rewrite_links(convert(src), md_dir), TITLES)
        body = re.sub(r"<table>", '<div class="table-wrap"><table>', body).replace("</table>", "</table></div>")
        key = out[:-5]
        title_m = re.search(r"^# (.+)$", src, re.M)
        title = title_m.group(1) if title_m else label
        extra = ""
        if key.startswith("day-"):
            n = int(key[4:])
            body, tasks = enhance_day(body)
            body = re.sub(r"(</h1>)", r"\1" + START_BOX.replace("\\", "\\\\"), body, count=1)
            prev_link = f'<a href="day-{n-1}.html">← Day {n-1}</a>' if n > 1 else '<a href="index.html">← Course home</a>'
            next_link = f'<a href="day-{n+1}.html">Day {n+1} →</a>' if n < 6 else '<a href="index.html">Course home →</a>'
            body += (f'<div class="pager">{prev_link}<button class="done-btn" id="doneBtn" type="button">'
                     f'Mark this day complete</button>{next_link}</div>')
            day_info.append({"n": n, "title": title.split(":", 1)[-1].strip(), "goal": extract(src, "Goal"),
                             "time": extract(src, "Time").split(" (")[0].split(":")[0].rstrip("."), "tasks": tasks})
        if key.startswith("diagram-"):
            extra = '<script src="https://cdn.jsdelivr.net/npm/mermaid@10.9.1/dist/mermaid.min.js"></script>'
            body = re.sub(r'(<div class="mermaid">.*?</div>)',
                          r'\1<p class="offline-note muted">This diagram needs an internet connection to draw. '
                          r'Above is its source. You can also view it on GitHub: <a href="' + GITHUB +
                          f'/blob/main/onboarding/{md_rel}">{md_rel}</a>.</p>', body, flags=re.S)
            body = '<p class="muted"><a href="index.html#diagrams">← All diagrams</a></p>' + body
        with open(os.path.join(SITE, out), "w", encoding="utf-8") as f:
            f.write(page_html(title + " · 35 Domino course", key, body, extra))
    write_index(day_info)
    print(f"Built {len(PAGES) + 1} pages in {os.path.relpath(SITE, ROOT)}/")


def write_index(days):
    cards = []
    for d in days:
        cards.append(f"""<a class="card" href="day-{d['n']}.html" data-progress="day-{d['n']}" data-total="{d['tasks']}">
<span class="badge"></span><h3>Day {d['n']}: {html.escape(d['title'])}</h3>
<p>{d['goal']}</p><p>{d['time']}</p>
<div class="progress"><div></div></div><span class="plabel muted"></span></a>""")
    diagram_titles = {
        "architecture": "Architecture: every class and how they connect",
        "request-flow": "One human turn (the main flow)",
        "data-model": "Data model and the save file",
        "computer-turn": "The computer's turn and help mode",
        "new-round": "Starting a game and a round",
        "round-end": "Passing and the end of a round",
        "save-game": "Saving a game",
        "resume-game": "Resuming a saved game",
    }
    dcards = "".join(f'<a class="card" href="diagram-{d}.html"><h3>{html.escape(t)}</h3></a>'
                     for d, t in diagram_titles.items())
    body = f"""<h1>35 Domino: 6-day onboarding course</h1>
<p>Over six days you'll learn how this C++ dominoes game works: what it does, how one turn flows through the code, how to trace it in the debugger, and how to change it yourself.</p>

<div class="box start-box">
<h4>Before Day 1</h4>
<p>Set up your computer by following <a href="onboarding.html#41-one-time-setup-on-windows-about-45-minutes">the setup guide, section 4.1</a> (about 45 minutes).</p>
<p>Each day, open that day's page below, work through it top to bottom, and click <b>Mark this day complete</b> at the end.</p>
</div>

<h2 id="days">The six days</h2>
<div class="cards">{''.join(cards)}</div>

<h2 id="reference">Reference</h2>
<div class="cards">
<a class="card" href="onboarding.html"><h3>Onboarding guide</h3><p>What the game does, the glossary, where to find things, setup and debugging.</p></a>
<a class="card" href="cpp-primer.html"><h3>C++ primer</h3><p>How programming ideas you already know are written in C++, with examples from this code.</p></a>
<a class="card" href="../simulator.html"><h3>Turn simulator ↗</h3><p>Step through one turn, class by class, with real data.</p></a>
</div>

<h2 id="diagrams">Diagrams</h2>
<div class="cards">{dcards}</div>
<p class="muted">Diagrams need an internet connection to draw. Progress and answers are saved in this browser only.</p>
"""
    with open(os.path.join(SITE, "index.html"), "w", encoding="utf-8") as f:
        f.write(page_html("35 Domino course", "index", body))


if __name__ == "__main__":
    build()
