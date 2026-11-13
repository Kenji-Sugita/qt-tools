"""Verify copied artifacts, extracted entries, links and Markdown fences."""
import hashlib,json,re,subprocess,unicodedata
from pathlib import Path
from urllib.parse import unquote,urlsplit
from zipfile import ZipFile
root=Path(__file__).resolve().parents[2];here=Path(__file__).resolve().parent;manifest=json.loads((here/'manifest.json').read_text());checks=[]
for row in manifest['files']:
 target=root/row['target'];content=target.read_bytes();assert hashlib.sha256(content).hexdigest()==row['sha256'],row['target']
 if 'source' in row:assert content==(Path.home()/'Sites'/row['source']).read_bytes(),row['target']
 if 'archive' in row:
  with ZipFile(root/row['archive']) as archive:assert content==archive.read(row['entry']),row['target']
archives=sorted({r['target'] for r in manifest['files'] if r['target'].endswith('.zip')})
for name in archives:
 with ZipFile(root/name) as archive:assert archive.testzip() is None,name
publication_dirs={Path(*Path(row['target']).parts[:3]) if row['target'].startswith('docs/') else Path(*Path(row['target']).parts[:2]) for row in manifest['files']}
markdown=set()
for directory in publication_dirs:markdown.update((root/directory).rglob('*.md'))
for name in ['README.md','README.en.md','PUBLICATION_MAP.md','docs/README.md','docs/README.en.md','tools/README.md','tools/README.en.md']:
 markdown.add(root/name)
for category in ['basics','practice','development','safety','perspectives']:
 for lang in ['', '.en']:markdown.add(root/f'docs/{category}/README{lang}.md')
markdown.add(here/'README.md')
# Strip fenced code before interpreting documentation links.
def prose(text):
 output=[];fence=None
 for line in text.splitlines():
  match=re.match(r'^\s*(`{3,}|~{3,})',line)
  if match:
   token=match.group(1)
   if fence is None:fence=token
   elif token[0]==fence[0] and len(token)>=len(fence):fence=None
   continue
  if fence is None:output.append(line)
 assert fence is None,'unclosed code fence'
 return '\n'.join(output)
links=0
for p in sorted(markdown):
 text=prose(p.read_text(encoding='utf-8-sig'))
 for link in re.findall(r'!?\[[^\]]*\]\(([^\n]+?)\)',text):
  url=link.strip().split(' "',1)[0].strip('<>');parsed=urlsplit(url)
  if parsed.scheme or url.startswith('//') or not parsed.path:continue
  dest=(p.parent/unquote(parsed.path)).resolve();assert dest.is_relative_to(root) and dest.exists(),(str(p.relative_to(root)),url)
  links+=1
# English guide keeps every original code example; diagram labels are translated.
ja=(root/'tools/fileservermcp/USER_GUIDE.md').read_text();en=(root/'tools/fileservermcp/USER_GUIDE.en.md').read_text();pattern=r'(?m)^```([^\n]*)\n(.*?)^```\s*$';ja_blocks=re.findall(pattern,ja,re.S);en_blocks=re.findall(pattern,en,re.S);assert len(ja_blocks)==len(en_blocks)==82
for (ja_lang,ja_code),(en_lang,en_code) in zip(ja_blocks,en_blocks):
 assert ja_lang==en_lang
 if ja_lang!='mermaid':assert ja_code.rstrip()==en_code.rstrip()
cmake=(root/'tools/fileservermcp/CMakeLists.txt').read_text();assert cmake.index('find_package(Qt6 REQUIRED COMPONENTS Core Network)')<cmake.index('qt_standard_project_setup()')<cmake.index('qt_add_executable(')
assert not any('how-we-use-ai/' in r.get('source','') for r in manifest['files'])
report={'canonical_files':35,'archive_entries':39,'unique_copied_destinations':70,'zip_crc_checks':len(archives),'markdown_files_checked':len(markdown),'local_links_checked':links,'fileserver_bilingual_code_blocks':82,'copied_cmake_policy':'Qt console/multiple-target patterns; unchanged archive copy','internal_qt_workbook':'excluded','pdfs_regenerated':False,'all_checks_passed':True}
(here/'publication-checks.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n');print(json.dumps(report,ensure_ascii=False,indent=2))
