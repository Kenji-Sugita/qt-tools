#!/usr/bin/env bash
set -euo pipefail

icpp_bin="$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
source_root="$2"
tmp_dir="$(mktemp -d)"
trap 'rm -rf "$tmp_dir"' EXIT

cling_path="${ICPP_CLING:-/usr/local/src/cling/build/bin/cling}"
export ICPP_HELP_LANGUAGE=en

expected_version="$(tr -d '[:space:]' < "$source_root/VERSION")"
actual_version="$("$icpp_bin" --version)"
if [[ "$actual_version" != "icpp $expected_version" ]]; then
    echo "Version mismatch: VERSION=$expected_version, binary=$actual_version" >&2
    exit 1
fi

if [[ ! -x "$cling_path" ]]; then
    echo "Skipping icpp integration test: cling not found at $cling_path" >&2
    exit 0
fi

cat > "$tmp_dir/input.icpp" <<'EOF'
.args
.b on
#include <iostream>
int add(int a, int b) { return a + b; }
std::cout << add(2, 3) << std::endl;
.p add(2, 3)
.no_such_command
.errors
.show
.save saved.cpp
.reset
.open saved.cpp
.run
.quit
EOF

(
    cd "$tmp_dir"
    "$icpp_bin" --engine cling --cling "$cling_path" < input.icpp > output.txt 2>&1
)

grep -q "engine: cling" "$tmp_dir/output.txt"
grep -q "Input buffer append: on" "$tmp_dir/output.txt"
grep -q "^5$" "$tmp_dir/output.txt"
grep -q "Unknown command: .no_such_command" "$tmp_dir/output.txt"
grep -q "2  int add" "$tmp_dir/output.txt"
grep -q "Saved: saved.cpp" "$tmp_dir/output.txt"
grep -q "Session reset." "$tmp_dir/output.txt"
grep -q "Opened: saved.cpp" "$tmp_dir/output.txt"

if [[ ! -s "$tmp_dir/saved.cpp" ]]; then
    echo "saved.cpp was not created" >&2
    exit 1
fi

cat > "$tmp_dir/order_a.cpp" <<'EOF'
int orderBase() { return 20; }
EOF
cat > "$tmp_dir/order_b.cpp" <<'EOF'
int orderUse() { return orderBase() + 2; }
EOF
cat > "$tmp_dir/order-editor.sh" <<EOF
#!/usr/bin/env bash
{
    grep 'order_a.cpp' "\$1"
    grep 'order_b.cpp' "\$1"
} > "\$1.reordered"
mv "\$1.reordered" "\$1"
EOF
chmod +x "$tmp_dir/order-editor.sh"
cat > "$tmp_dir/order.icpp" <<EOF
.add $tmp_dir/order_b.cpp
.add $tmp_dir/order_a.cpp
.r edit
.p orderUse()
.quit
EOF

ICPP_EDITOR="$tmp_dir/order-editor.sh" "$icpp_bin" --engine cling --cling "$cling_path" \
    < "$tmp_dir/order.icpp" > "$tmp_dir/order.txt" 2>&1
grep -q "Registered file order updated." "$tmp_dir/order.txt"
grep -q "1  $tmp_dir/order_a.cpp" "$tmp_dir/order.txt"
grep -q "2  $tmp_dir/order_b.cpp" "$tmp_dir/order.txt"
grep -q "^22$" "$tmp_dir/order.txt"

cat > "$tmp_dir/help.icpp" <<'EOF'
.help
.quit
EOF

"$icpp_bin" --engine cling --cling "$cling_path" < "$tmp_dir/help.icpp" > "$tmp_dir/help.txt" 2>&1
grep -q ".loadlib <path>" "$tmp_dir/help.txt"
grep -q ".! <command>" "$tmp_dir/help.txt"
grep -q ".gen" "$tmp_dir/help.txt"
grep -q ".eval, .x <code>" "$tmp_dir/help.txt"
grep -q ".append, .a <code>" "$tmp_dir/help.txt"
grep -q ".buffer, .b \\[on|off\\]" "$tmp_dir/help.txt"
grep -q ".status, .st" "$tmp_dir/help.txt"
grep -q ".doctor" "$tmp_dir/help.txt"
grep -q ".generated" "$tmp_dir/help.txt"
grep -q ".clean" "$tmp_dir/help.txt"
grep -q ".autogen \\[on|off\\]" "$tmp_dir/help.txt"
grep -q ".edit, .e \\[file|number\\]" "$tmp_dir/help.txt"
grep -q ".i \\[path|edit\\]" "$tmp_dir/help.txt"
grep -q ".defs" "$tmp_dir/help.txt"
grep -q ".uiinfo <file.ui>" "$tmp_dir/help.txt"
grep -q ".preview, .pv <file.ui>" "$tmp_dir/help.txt"
grep -q ".inspect" "$tmp_dir/help.txt"
grep -q ".designer, .de <form|file.ui>" "$tmp_dir/help.txt"
grep -q ".linguist, .li <file.ts>" "$tmp_dir/help.txt"
grep -q ".qrc \\[text|creator\\] <file.qrc>" "$tmp_dir/help.txt"
grep -q ".qtc <file.qrc>" "$tmp_dir/help.txt"
grep -q ".template <kind> \\[base\\]" "$tmp_dir/help.txt"
grep -q ".quiet \\[on|off\\]" "$tmp_dir/help.txt"
grep -q ".newname \\[lower|asis\\]" "$tmp_dir/help.txt"

env -u ICPP_HELP_LANGUAGE "$icpp_bin" --engine cling --cling "$cling_path" \
    < "$tmp_dir/help.icpp" > "$tmp_dir/help_ja.txt" 2>&1
grep -q "セッション" "$tmp_dir/help_ja.txt"

ICPP_HELP_LANGUAGE=ja "$icpp_bin" --engine cling --cling "$cling_path" \
    < "$tmp_dir/help.icpp" > "$tmp_dir/help_env_ja.txt" 2>&1
grep -q "編集バッファ" "$tmp_dir/help_env_ja.txt"

"$icpp_bin" --engine cling --cling "$cling_path" --help-language ja \
    < "$tmp_dir/help.icpp" > "$tmp_dir/help_cli_ja.txt" 2>&1
grep -q "登録ファイル" "$tmp_dir/help_cli_ja.txt"

cat > "$tmp_dir/template.icpp" <<'EOF'
.quiet off
.newname lower
.template -h
.template widget sample_widget
Y
.template widget sample_widget
.template widget QuitButton
Y
.template ui QuitButtonForm
Y
.newname asis
.template widget AsisButton
Y
.template ui AsisForm
Y
.quit
EOF

(
    cd "$tmp_dir"
    "$icpp_bin" --engine cling --cling "$cling_path" < template.icpp > template.txt 2>&1
)
grep -q ".template <kind> \\[base\\]" "$tmp_dir/template.txt"
grep -q "Files to create:" "$tmp_dir/template.txt"
grep -q "sample_widget.h" "$tmp_dir/template.txt"
grep -q "sample_widget.cpp" "$tmp_dir/template.txt"
grep -q "class: SampleWidget" "$tmp_dir/template.txt"
grep -q "Created: sample_widget.h" "$tmp_dir/template.txt"
grep -q "Created: sample_widget.cpp" "$tmp_dir/template.txt"
grep -q "Next steps:" "$tmp_dir/template.txt"
grep -q "File already exists; not creating templates:" "$tmp_dir/template.txt"
grep -q "class SampleWidget" "$tmp_dir/sample_widget.h"
grep -q "Q_OBJECT" "$tmp_dir/sample_widget.h"
grep -q "#include \"moc_sample_widget.cpp\"" "$tmp_dir/sample_widget.cpp"
grep -q "Created: quitbutton.h" "$tmp_dir/template.txt"
grep -q "class: QuitButton" "$tmp_dir/template.txt"
grep -q "class QuitButton" "$tmp_dir/quitbutton.h"
grep -q "QuitButton::QuitButton" "$tmp_dir/quitbutton.cpp"
grep -q "Created: quitbuttonform.ui" "$tmp_dir/template.txt"
grep -q "form class: QuitButtonForm" "$tmp_dir/template.txt"
grep -q "<class>QuitButtonForm</class>" "$tmp_dir/quitbuttonform.ui"
grep -q "Created: AsisButton.h" "$tmp_dir/template.txt"
grep -q "class: AsisButton" "$tmp_dir/template.txt"
grep -q "class AsisButton" "$tmp_dir/AsisButton.h"
grep -q "AsisButton::AsisButton" "$tmp_dir/AsisButton.cpp"
grep -q "Created: AsisForm.ui" "$tmp_dir/template.txt"
grep -q "form class: AsisForm" "$tmp_dir/template.txt"
grep -q "<class>AsisForm</class>" "$tmp_dir/AsisForm.ui"

cat > "$tmp_dir/form.ui" <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>Form</class>
 <widget class="QWidget" name="Form">
  <layout class="QVBoxLayout" name="verticalLayout">
   <item>
    <widget class="QLabel" name="titleLabel"/>
   </item>
   <item>
    <widget class="QLineEdit" name="nameEdit"/>
   </item>
   <item>
    <widget class="QPushButton" name="okButton"/>
   </item>
  </layout>
  <action name="actionOpen"/>
 </widget>
</ui>
EOF

cat > "$tmp_dir/uiinfo.icpp" <<'EOF'
.uiinfo form.ui
.quit
EOF

(
    cd "$tmp_dir"
    "$icpp_bin" --engine cling --cling "$cling_path" < uiinfo.icpp > uiinfo.txt 2>&1
)
grep -q "class: Form" "$tmp_dir/uiinfo.txt"
grep -q "base: QWidget" "$tmp_dir/uiinfo.txt"
grep -q "QLabel .*titleLabel" "$tmp_dir/uiinfo.txt"
grep -q "QLineEdit .*nameEdit" "$tmp_dir/uiinfo.txt"
grep -q "QPushButton .*okButton" "$tmp_dir/uiinfo.txt"
grep -q "QVBoxLayout .*verticalLayout" "$tmp_dir/uiinfo.txt"
grep -q "QAction .*actionOpen" "$tmp_dir/uiinfo.txt"

cat > "$tmp_dir/designer.sh" <<'EOF'
#!/usr/bin/env bash
printf '<!-- edited by designer -->\n' >> "$1"
EOF
chmod +x "$tmp_dir/designer.sh"

cat > "$tmp_dir/designer.icpp" <<'EOF'
.newname lower
.designer form.ui
.de form.ui
.designer new_form.ui
Y
.designer DesignerButtonForm
Y
.designer WifgetForm.ui
Y
.newname asis
.designer AsisDesignerForm
Y
.designer missing/new_form.ui
.designer form.txt
.quit
EOF
cp "$tmp_dir/form.ui" "$tmp_dir/form.txt"

(
    cd "$tmp_dir"
    ICPP_DESIGNER="$tmp_dir/designer.sh" "$icpp_bin" --engine cling --cling "$cling_path" \
        < designer.icpp > designer.txt 2>&1
)
grep -q "Opened with Designer: form.ui" "$tmp_dir/designer.txt"
grep -q "Created .ui file: new_form.ui" "$tmp_dir/designer.txt"
grep -q "Opened with Designer: new_form.ui" "$tmp_dir/designer.txt"
grep -q "Created .ui file: designerbuttonform.ui" "$tmp_dir/designer.txt"
grep -q "Opened with Designer: designerbuttonform.ui" "$tmp_dir/designer.txt"
grep -q "Created .ui file: wifgetform.ui" "$tmp_dir/designer.txt"
grep -q "Opened with Designer: wifgetform.ui" "$tmp_dir/designer.txt"
grep -q "Created .ui file: AsisDesignerForm.ui" "$tmp_dir/designer.txt"
grep -q "Opened with Designer: AsisDesignerForm.ui" "$tmp_dir/designer.txt"
grep -q "After saving in Designer, run .gen" "$tmp_dir/designer.txt"
grep -q "missing/new_form.ui: Parent directory not found." "$tmp_dir/designer.txt"
grep -q "form.txt: Not a .ui file." "$tmp_dir/designer.txt"
grep -q "<class>NewForm</class>" "$tmp_dir/new_form.ui"
grep -q "<class>DesignerButtonForm</class>" "$tmp_dir/designerbuttonform.ui"
grep -q "<class>WifgetForm</class>" "$tmp_dir/wifgetform.ui"
grep -q "<class>AsisDesignerForm</class>" "$tmp_dir/AsisDesignerForm.ui"

cat > "$tmp_dir/designer-fail.icpp" <<'EOF'
.designer form.ui
.quit
EOF

(
    cd "$tmp_dir"
    ICPP_DESIGNER="$tmp_dir/missing-designer" "$icpp_bin" --engine cling --cling "$cling_path" \
        < designer-fail.icpp > designer-fail.txt 2>&1
)
grep -q "Cannot start Designer:" "$tmp_dir/designer-fail.txt"
if grep -q "Opened with Designer: form.ui" "$tmp_dir/designer-fail.txt"; then
    echo "designer start failure was reported as success" >&2
    exit 1
fi

cat > "$tmp_dir/app_ja.ts" <<'EOF'
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ja_JP">
<context>
    <name>Widget</name>
    <message>
        <source>Hello</source>
        <translation>こんにちは</translation>
    </message>
</context>
</TS>
EOF

cat > "$tmp_dir/linguist.sh" <<'EOF'
#!/usr/bin/env bash
printf '<!-- edited by linguist -->\n' >> "$1"
EOF
chmod +x "$tmp_dir/linguist.sh"

cat > "$tmp_dir/linguist.icpp" <<'EOF'
.linguist app_ja.ts
.li app_ja.ts
.linguist missing.ts
.linguist form.ui
.quit
EOF

(
    cd "$tmp_dir"
    ICPP_LINGUIST="$tmp_dir/linguist.sh" "$icpp_bin" --engine cling --cling "$cling_path" \
        < linguist.icpp > linguist.txt 2>&1
)
grep -q "Opened with Linguist: app_ja.ts" "$tmp_dir/linguist.txt"
grep -q "After saving in Linguist, run .! lrelease app_ja.ts" "$tmp_dir/linguist.txt"
grep -q "missing.ts: .ts file not found." "$tmp_dir/linguist.txt"
grep -q "form.ui: Not a .ts file." "$tmp_dir/linguist.txt"

cat > "$tmp_dir/linguist-fail.icpp" <<'EOF'
.linguist app_ja.ts
.quit
EOF

(
    cd "$tmp_dir"
    ICPP_LINGUIST="$tmp_dir/missing-linguist" "$icpp_bin" --engine cling --cling "$cling_path" \
        < linguist-fail.icpp > linguist-fail.txt 2>&1
)
grep -q "Cannot start Linguist:" "$tmp_dir/linguist-fail.txt"
if grep -q "Opened with Linguist: app_ja.ts" "$tmp_dir/linguist-fail.txt"; then
    echo "linguist start failure was reported as success" >&2
    exit 1
fi

cat > "$tmp_dir/resources.qrc" <<'EOF'
<RCC>
    <qresource prefix="/">
        <file>message.txt</file>
    </qresource>
</RCC>
EOF

cat > "$tmp_dir/editor-qrc.sh" <<'EOF'
#!/usr/bin/env bash
printf '<!-- edited by text editor -->\n' >> "$1"
EOF
chmod +x "$tmp_dir/editor-qrc.sh"

cat > "$tmp_dir/qtcreator.sh" <<'EOF'
#!/usr/bin/env bash
printf '<!-- edited by qt creator -->\n' >> "$1"
EOF
chmod +x "$tmp_dir/qtcreator.sh"

cat > "$tmp_dir/qrc.icpp" <<'EOF'
.qrc resources.qrc
.qrc text resources.qrc
.qrc creator resources.qrc
.qtc resources.qrc
.qrc missing.qrc
.qrc form.ui
.quit
EOF

(
    cd "$tmp_dir"
    ICPP_EDITOR="$tmp_dir/editor-qrc.sh" ICPP_QTCREATOR="$tmp_dir/qtcreator.sh" \
        "$icpp_bin" --engine cling --cling "$cling_path" < qrc.icpp > qrc.txt 2>&1
)
grep -q "Edited with text editor: resources.qrc" "$tmp_dir/qrc.txt"
grep -q "Edited with Qt Creator: resources.qrc" "$tmp_dir/qrc.txt"
grep -q "If the .qrc file changed, run .gen" "$tmp_dir/qrc.txt"
grep -q "missing.qrc: .qrc file not found." "$tmp_dir/qrc.txt"
grep -q "form.ui: Not a .qrc file." "$tmp_dir/qrc.txt"
grep -q "edited by text editor" "$tmp_dir/resources.qrc"
grep -q "edited by qt creator" "$tmp_dir/resources.qrc"

cat > "$tmp_dir/widget.h" <<'EOF'
class WorkflowWidget {
    Q_OBJECT
};
EOF
touch "$tmp_dir/moc_widget.cpp" "$tmp_dir/ui_form.h" "$tmp_dir/qrc_resources.cpp" "$tmp_dir/app_ja.qm"

cat > "$tmp_dir/generated_tools.icpp" <<'EOF'
.generated
.doctor
.clean
N
.clean
Y
.generated
.quit
EOF

(
    cd "$tmp_dir"
    "$icpp_bin" --engine cling --cling "$cling_path" < generated_tools.icpp > generated_tools.txt 2>&1
)
grep -q "generated files:" "$tmp_dir/generated_tools.txt"
grep -q "moc_widget.cpp" "$tmp_dir/generated_tools.txt"
grep -q "qrc_resources.cpp" "$tmp_dir/generated_tools.txt"
grep -q "ui_form.h" "$tmp_dir/generated_tools.txt"
grep -q "app_ja.qm" "$tmp_dir/generated_tools.txt"
grep -q "icpp doctor" "$tmp_dir/generated_tools.txt"
grep -q "Generated files to delete:" "$tmp_dir/generated_tools.txt"
grep -q "Delete canceled." "$tmp_dir/generated_tools.txt"
grep -q "Deleted: moc_widget.cpp" "$tmp_dir/generated_tools.txt"
grep -q "(none)" "$tmp_dir/generated_tools.txt"
test ! -e "$tmp_dir/moc_widget.cpp"
test ! -e "$tmp_dir/ui_form.h"
test ! -e "$tmp_dir/qrc_resources.cpp"
test ! -e "$tmp_dir/app_ja.qm"
test -e "$tmp_dir/widget.h"
test -e "$tmp_dir/form.ui"
test -e "$tmp_dir/resources.qrc"
test -e "$tmp_dir/app_ja.ts"

cat > "$tmp_dir/qtcreator-fail.sh" <<'EOF'
#!/usr/bin/env bash
exit 7
EOF
chmod +x "$tmp_dir/qtcreator-fail.sh"

cat > "$tmp_dir/qrc-fail.icpp" <<'EOF'
.qrc creator resources.qrc
.quit
EOF

(
    cd "$tmp_dir"
    ICPP_QTCREATOR="$tmp_dir/qtcreator-fail.sh" "$icpp_bin" --engine cling --cling "$cling_path" \
        < qrc-fail.icpp > qrc-fail.txt 2>&1
)
grep -q "Qt Creator exited with status 7:" "$tmp_dir/qrc-fail.txt"
if grep -q "Edited with Qt Creator: resources.qrc" "$tmp_dir/qrc-fail.txt"; then
    echo "qtcreator failure was reported as success" >&2
    exit 1
fi

cat > "$tmp_dir/external.icpp" <<'EOF'
.! echo external-ok
.!echo compact-ok
.!    spaced-ok
.! exit 7
.errors
.quit
EOF

"$icpp_bin" --engine cling --cling "$cling_path" < "$tmp_dir/external.icpp" > "$tmp_dir/external.txt" 2>&1
grep -q "external-ok" "$tmp_dir/external.txt"
grep -q "compact-ok" "$tmp_dir/external.txt"
grep -q "spaced-ok" "$tmp_dir/external.txt"
grep -q "External command exited with status 7: exit 7" "$tmp_dir/external.txt"

mkdir -p "$tmp_dir/include-test"
cat > "$tmp_dir/include-test/local_value.h" <<'EOF'
inline int localValue() { return 19; }
EOF

cat > "$tmp_dir/include_path.icpp" <<EOF
.i $tmp_dir/include-test
.i
#include "local_value.h"
.p localValue()
.quit
EOF

"$icpp_bin" --engine cling --cling "$cling_path" < "$tmp_dir/include_path.icpp" > "$tmp_dir/include_path.txt" 2>&1
grep -q "Added include path: $tmp_dir/include-test" "$tmp_dir/include_path.txt"
grep -q "Include paths:" "$tmp_dir/include_path.txt"
grep -q "$tmp_dir/include-test" "$tmp_dir/include_path.txt"
grep -q "^19$" "$tmp_dir/include_path.txt"

mkdir -p "$tmp_dir/gen-bin"
cat > "$tmp_dir/gen-bin/run_all" <<EOF
#!/usr/bin/env bash
echo generated-ok
touch "$tmp_dir/generated-marker"
EOF
chmod +x "$tmp_dir/gen-bin/run_all"

cat > "$tmp_dir/gen.icpp" <<'EOF'
.b on
int generatedValue() { return 23; }
.gen
.p generatedValue()
.quit
Y
EOF

PATH="$tmp_dir/gen-bin:$PATH" "$icpp_bin" --engine cling --cling "$cling_path" \
    < "$tmp_dir/gen.icpp" > "$tmp_dir/gen.txt" 2>&1
grep -q "generated-ok" "$tmp_dir/gen.txt"
grep -q "^23$" "$tmp_dir/gen.txt"
test -f "$tmp_dir/generated-marker"

cat > "$tmp_dir/status.icpp" <<'EOF'
.newname lower
.status
.buffer on
.autogen
.autogen on
.newname asis
.status
.quit
EOF

"$icpp_bin" --engine cling --cling "$cling_path" < "$tmp_dir/status.icpp" > "$tmp_dir/status.txt" 2>&1
grep -q "engine: cling" "$tmp_dir/status.txt"
grep -q "help language: en" "$tmp_dir/status.txt"
grep -q "input buffer append: off" "$tmp_dir/status.txt"
grep -q "input buffer append: on" "$tmp_dir/status.txt"
grep -q "Autogen: off" "$tmp_dir/status.txt"
grep -q "Autogen: on" "$tmp_dir/status.txt"
grep -q "autogen: on" "$tmp_dir/status.txt"
grep -q "new file name: lower" "$tmp_dir/status.txt"
grep -q "new file name: asis" "$tmp_dir/status.txt"

cat > "$tmp_dir/autogen.cpp" <<'EOF'
#define Q_OBJECT
class AutogenHarness {
    Q_OBJECT
};
EOF
cat > "$tmp_dir/gen-bin/run_all" <<EOF
#!/usr/bin/env bash
echo autogen-run
touch "$tmp_dir/autogen-marker"
EOF
chmod +x "$tmp_dir/gen-bin/run_all"

cat > "$tmp_dir/autogen.icpp" <<EOF
.open $tmp_dir/autogen.cpp
.autogen on
.r
.quit
EOF

PATH="$tmp_dir/gen-bin:$PATH" "$icpp_bin" --engine cling --cling "$cling_path" \
    < "$tmp_dir/autogen.icpp" > "$tmp_dir/autogen.txt" 2>&1
grep -q "Autogen: run_all" "$tmp_dir/autogen.txt"
grep -q "autogen-run" "$tmp_dir/autogen.txt"
test -f "$tmp_dir/autogen-marker"

cat > "$tmp_dir/eval.icpp" <<'EOF'
.b on
#include <iostream>
int visible_value = 5;
.x int hidden_value = visible_value * 3;
.eval std::cout << hidden_value << std::endl;
.a int appended_value = hidden_value + 1;
.show
.quit
Y
EOF

"$icpp_bin" --engine cling --cling "$cling_path" < "$tmp_dir/eval.icpp" > "$tmp_dir/eval.txt" 2>&1
grep -q "^15$" "$tmp_dir/eval.txt"
grep -q "2  int visible_value = 5;" "$tmp_dir/eval.txt"
grep -q "3  int appended_value = hidden_value + 1;" "$tmp_dir/eval.txt"
if grep -q "1  int hidden_value" "$tmp_dir/eval.txt"; then
    echo ".x/.eval unexpectedly appended code to the edit buffer" >&2
    exit 1
fi

cat > "$tmp_dir/buffer.icpp" <<'EOF'
.b on
int kept_value = 10;
.b
int scratch_value = kept_value + 5;
.show
.buffer
.b
int kept_again = scratch_value + 5;
.show
.buffer off
int scratch_again = kept_again + 5;
.show
.buffer on
.quit
Y
EOF

"$icpp_bin" --engine cling --cling "$cling_path" < "$tmp_dir/buffer.icpp" > "$tmp_dir/buffer.txt" 2>&1
grep -q "Input buffer append: off" "$tmp_dir/buffer.txt"
grep -q "Input buffer append: on" "$tmp_dir/buffer.txt"
grep -q "1  int kept_value = 10;" "$tmp_dir/buffer.txt"
grep -q "2  int kept_again = scratch_value + 5;" "$tmp_dir/buffer.txt"
if grep -q "scratch_again" "$tmp_dir/buffer.txt"; then
    echo ".buffer off unexpectedly appended normal input" >&2
    exit 1
fi

cat > "$tmp_dir/warning-editor.sh" <<'EOF'
#!/usr/bin/env bash
cat > "$1" <<'CPP'
#define Q_OBJECT
class WarningHarness {
    Q_OBJECT
};
CPP
EOF
chmod +x "$tmp_dir/warning-editor.sh"

cat > "$tmp_dir/generated_warning.icpp" <<'EOF'
.edit
.quit
Y
EOF

ICPP_EDITOR="$tmp_dir/warning-editor.sh" "$icpp_bin" --engine cling --cling "$cling_path" \
    < "$tmp_dir/generated_warning.icpp" > "$tmp_dir/generated_warning.txt" 2>&1
grep -q "Warning: Qt generated files may be needed. Run .gen" "$tmp_dir/generated_warning.txt"

mkdir -p "$tmp_dir/qrc-edit"
cat > "$tmp_dir/qrc-edit/resources.qrc" <<'EOF'
<RCC>
  <qresource prefix="/">
  </qresource>
</RCC>
EOF
cat > "$tmp_dir/qrc_edit.icpp" <<'EOF'
.e resources.qrc
.quit
EOF

(
    cd "$tmp_dir/qrc-edit"
    ICPP_EDITOR=true "$icpp_bin" --engine cling --cling "$cling_path" \
        < "$tmp_dir/qrc_edit.icpp" > "$tmp_dir/qrc_edit.txt" 2>&1
)
grep -q "Warning: Qt generated files may be needed. Run .gen" "$tmp_dir/qrc_edit.txt"
grep -q "No C++ files or edit buffer to run. Use .e <file.cpp> or .add <file.cpp>, then run .gen." "$tmp_dir/qrc_edit.txt"

cat > "$tmp_dir/unsaved.icpp" <<'EOF'
.b on
int unsaved_value = 42;
.quit
N
.show
.quit
Y
EOF

"$icpp_bin" --engine cling --cling "$cling_path" < "$tmp_dir/unsaved.icpp" > "$tmp_dir/unsaved.txt" 2>&1
grep -q "Quit without saving? \\[Y/N\\]" "$tmp_dir/unsaved.txt"
grep -q "1  int unsaved_value = 42;" "$tmp_dir/unsaved.txt"

cat > "$tmp_dir/saved_quit.icpp" <<'EOF'
.b on
int saved_value = 42;
.save saved_quit.cpp
.quit
EOF

"$icpp_bin" --engine cling --cling "$cling_path" < "$tmp_dir/saved_quit.icpp" > "$tmp_dir/saved_quit.txt" 2>&1
if grep -q "Quit without saving" "$tmp_dir/saved_quit.txt"; then
    echo "saved buffer unexpectedly prompted before quit" >&2
    exit 1
fi

cat > "$tmp_dir/save_a.cpp" <<'EOF'
int save_value = 1;
EOF
cat > "$tmp_dir/save_b.cpp" <<'EOF'
int other_value = 2;
EOF

cat > "$tmp_dir/safe_save.icpp" <<EOF
.open $tmp_dir/save_a.cpp
.add $tmp_dir/save_b.cpp
.b on
int edited_value = 3;
.save
N
.save
Y
.quit
EOF

"$icpp_bin" --engine cling --cling "$cling_path" < "$tmp_dir/safe_save.icpp" > "$tmp_dir/safe_save.txt" 2>&1
grep -q "Warning: multiple files are in use. Save edit buffer to $tmp_dir/save_a.cpp? \\[Y/N\\]" "$tmp_dir/safe_save.txt"
grep -q "edited_value" "$tmp_dir/save_a.cpp"

cat > "$tmp_dir/open_a.cpp" <<'EOF'
int open_a = 1;
EOF
cat > "$tmp_dir/open_b.cpp" <<'EOF'
int open_b = 2;
EOF

cat > "$tmp_dir/safe_open.icpp" <<EOF
.open $tmp_dir/open_a.cpp
.b on
int unsaved_open = 3;
.open $tmp_dir/open_b.cpp
N
.show
.open $tmp_dir/open_b.cpp
Y
.show
.quit
EOF

"$icpp_bin" --engine cling --cling "$cling_path" < "$tmp_dir/safe_open.icpp" > "$tmp_dir/safe_open.txt" 2>&1
grep -q "Warning: edit buffer has unsaved changes and will be replaced. Continue? \\[Y/N\\]" "$tmp_dir/safe_open.txt"
grep -q "unsaved_open" "$tmp_dir/safe_open.txt"
grep -q "int open_b = 2;" "$tmp_dir/safe_open.txt"

cat > "$tmp_dir/safe_discard_reset.icpp" <<'EOF'
.b on
int discard_value = 1;
.discard
N
.show
.reset
N
.show
.discard
Y
.quit
EOF

"$icpp_bin" --engine cling --cling "$cling_path" < "$tmp_dir/safe_discard_reset.icpp" > "$tmp_dir/safe_discard_reset.txt" 2>&1
grep -q "Warning: edit buffer has unsaved changes and will be discarded. Continue? \\[Y/N\\]" "$tmp_dir/safe_discard_reset.txt"
grep -q "Warning: edit buffer has unsaved changes. Reset the session? \\[Y/N\\]" "$tmp_dir/safe_discard_reset.txt"
grep -q "1  int discard_value = 1;" "$tmp_dir/safe_discard_reset.txt"

cat > "$tmp_dir/simple_class.cpp" <<'EOF'
class SimpleClass {
public:
    int getValue() const { return 21; }
};
EOF

cat > "$tmp_dir/use_simple_class.cpp" <<'EOF'
int makeValue() {
    SimpleClass value;
    return value.getValue() * 2;
}
EOF

cat > "$tmp_dir/defs_noise.cpp" <<'EOF'
void setupConnections() {
    QObject::connect(sizeSlider, &QSlider::valueChanged, this, [this]() {
        updatePreview();
    });
}
EOF

cat > "$tmp_dir/access_widget.h" <<'EOF'
class AccessWidget {
public:
    AccessWidget();
    void showPublic();

protected:
    void showProtected();

private:
    void showPrivate();
};
EOF

cat > "$tmp_dir/access_widget.cpp" <<'EOF'
#include "access_widget.h"

AccessWidget::AccessWidget() {}
void AccessWidget::showPublic() {}
void AccessWidget::showProtected() {}
void AccessWidget::showPrivate() {}
EOF

cat > "$tmp_dir/files.icpp" <<EOF
.add $tmp_dir/simple_class.cpp
.add $tmp_dir/use_simple_class.cpp
.files
.r
.p makeValue()
.drop 1
.files
.clearfiles
.files
.quit
EOF

"$icpp_bin" --engine cling --cling "$cling_path" < "$tmp_dir/files.icpp" > "$tmp_dir/files.txt" 2>&1
grep -q "Added: $tmp_dir/simple_class.cpp" "$tmp_dir/files.txt"
grep -q "Added: $tmp_dir/use_simple_class.cpp" "$tmp_dir/files.txt"
grep -q "^42$" "$tmp_dir/files.txt"
grep -q "Dropped: $tmp_dir/simple_class.cpp" "$tmp_dir/files.txt"
grep -q "Registered files cleared." "$tmp_dir/files.txt"
grep -q "No registered files." "$tmp_dir/files.txt"

cat > "$tmp_dir/defs.icpp" <<EOF
.add $tmp_dir/simple_class.cpp
.add $tmp_dir/use_simple_class.cpp
.add $tmp_dir/defs_noise.cpp
.add $tmp_dir/access_widget.cpp
.b on
int bufferOnlyDefinition() { return 5; }
bufferOnlyDefinition();
.defs
.quit
Y
EOF

"$icpp_bin" --engine cling --cling "$cling_path" < "$tmp_dir/defs.icpp" > "$tmp_dir/defs.txt" 2>&1
grep -q "registered files:" "$tmp_dir/defs.txt"
grep -q "class SimpleClass" "$tmp_dir/defs.txt"
grep -q "function int makeValue()" "$tmp_dir/defs.txt"
grep -q "public function AccessWidget::AccessWidget()" "$tmp_dir/defs.txt"
grep -q "public function void AccessWidget::showPublic()" "$tmp_dir/defs.txt"
grep -q "protected function void AccessWidget::showProtected()" "$tmp_dir/defs.txt"
grep -q "private function void AccessWidget::showPrivate()" "$tmp_dir/defs.txt"
grep -q "edit buffer:" "$tmp_dir/defs.txt"
grep -q "function int bufferOnlyDefinition()" "$tmp_dir/defs.txt"
if grep -q "function bufferOnlyDefinition();" "$tmp_dir/defs.txt"; then
    echo ".defs unexpectedly listed a function call" >&2
    exit 1
fi
if grep -q "function QObject::connect" "$tmp_dir/defs.txt"; then
    echo ".defs unexpectedly listed a function body call" >&2
    exit 1
fi

cat > "$tmp_dir/edit_registered_editor.sh" <<'EOF'
#!/usr/bin/env bash
cat > "$1" <<'CPP'
class SimpleClass {
public:
    int getValue() const { return 30; }
};
CPP
EOF
chmod +x "$tmp_dir/edit_registered_editor.sh"

cat > "$tmp_dir/edit_registered.icpp" <<EOF
.add $tmp_dir/simple_class.cpp
.add $tmp_dir/use_simple_class.cpp
.e 1
.p makeValue()
.quit
EOF

ICPP_EDITOR="$tmp_dir/edit_registered_editor.sh" "$icpp_bin" --engine cling --cling "$cling_path" \
    < "$tmp_dir/edit_registered.icpp" > "$tmp_dir/edit_registered.txt" 2>&1
grep -q "Edited: simple_class.cpp" "$tmp_dir/edit_registered.txt"
grep -q "^60$" "$tmp_dir/edit_registered.txt"
grep -q "return 30;" "$tmp_dir/simple_class.cpp"

cat > "$tmp_dir/create_registered_editor.sh" <<'EOF'
#!/usr/bin/env bash
cat > "$1" <<'CPP'
int createdRegisteredValue() { return 77; }
CPP
EOF
chmod +x "$tmp_dir/create_registered_editor.sh"

cat > "$tmp_dir/create_registered.icpp" <<EOF
.e $tmp_dir/created_registered.cpp
.files
.p createdRegisteredValue()
.quit
EOF

ICPP_EDITOR="$tmp_dir/create_registered_editor.sh" "$icpp_bin" --engine cling --cling "$cling_path" \
    < "$tmp_dir/create_registered.icpp" > "$tmp_dir/create_registered.txt" 2>&1
grep -q "Created: $tmp_dir/created_registered.cpp" "$tmp_dir/create_registered.txt"
grep -q "Added: $tmp_dir/created_registered.cpp" "$tmp_dir/create_registered.txt"
grep -q "1  $tmp_dir/created_registered.cpp" "$tmp_dir/create_registered.txt"
grep -q "^77$" "$tmp_dir/create_registered.txt"

cat > "$tmp_dir/create_header_editor.sh" <<'EOF'
#!/usr/bin/env bash
cat > "$1" <<'CPP'
#pragma once
inline int headerOnlyValue() { return 88; }
CPP
EOF
chmod +x "$tmp_dir/create_header_editor.sh"

cat > "$tmp_dir/create_header.icpp" <<EOF
.e $tmp_dir/created_header.h
.files
.quit
EOF

ICPP_EDITOR="$tmp_dir/create_header_editor.sh" "$icpp_bin" --engine cling --cling "$cling_path" \
    < "$tmp_dir/create_header.icpp" > "$tmp_dir/create_header.txt" 2>&1
grep -q "Created: $tmp_dir/created_header.h" "$tmp_dir/create_header.txt"
grep -q "Edited: created_header.h" "$tmp_dir/create_header.txt"
grep -q "No registered files." "$tmp_dir/create_header.txt"
if grep -q "Added: $tmp_dir/created_header.h" "$tmp_dir/create_header.txt"; then
    echo ".e unexpectedly auto-registered a header" >&2
    exit 1
fi

cat > "$tmp_dir/moc_placeholder_header.h" <<'EOF'
#pragma once
class MocPlaceholder {
public:
    int value() const { return 91; }
};
EOF
cat > "$tmp_dir/moc_placeholder_editor.sh" <<'EOF'
#!/usr/bin/env bash
cat > "$1" <<'CPP'
#include "moc_placeholder_header.h"
int mocPlaceholderValue() {
    MocPlaceholder value;
    return value.value();
}
#include "moc_moc_placeholder_header.cpp"
CPP
EOF
chmod +x "$tmp_dir/moc_placeholder_editor.sh"

cat > "$tmp_dir/moc_placeholder.icpp" <<EOF
.i $tmp_dir
.e $tmp_dir/moc_placeholder.cpp
.p mocPlaceholderValue()
.quit
EOF

ICPP_EDITOR="$tmp_dir/moc_placeholder_editor.sh" "$icpp_bin" --engine cling --cling "$cling_path" \
    < "$tmp_dir/moc_placeholder.icpp" > "$tmp_dir/moc_placeholder.txt" 2>&1
grep -q "Created placeholder: $tmp_dir/moc_moc_placeholder_header.cpp" "$tmp_dir/moc_placeholder.txt"
grep -q "^91$" "$tmp_dir/moc_placeholder.txt"
test -f "$tmp_dir/moc_moc_placeholder_header.cpp"

mkdir -p "$tmp_dir/clipboard-bin"
cat > "$tmp_dir/clipboard-bin/pbpaste" <<'EOF'
#!/usr/bin/env bash
printf 'int clipboard_value = 7;\nint doubled_clipboard_value() { return clipboard_value * 2; }\n'
EOF
cat > "$tmp_dir/clipboard-bin/pbcopy" <<EOF
#!/usr/bin/env bash
cat > "$tmp_dir/copied-buffer.cpp"
EOF
chmod +x "$tmp_dir/clipboard-bin/pbpaste" "$tmp_dir/clipboard-bin/pbcopy"

cat > "$tmp_dir/clipboard.icpp" <<'EOF'
.paste
.show
.copy
.r
.p doubled_clipboard_value()
.quit
Y
EOF

PATH="$tmp_dir/clipboard-bin:$PATH" "$icpp_bin" --engine cling --cling "$cling_path" \
    < "$tmp_dir/clipboard.icpp" > "$tmp_dir/clipboard.txt" 2>&1
grep -q "Pasted 2 lines." "$tmp_dir/clipboard.txt"
grep -q "Copied 2 lines." "$tmp_dir/clipboard.txt"
grep -q "1  int clipboard_value = 7;" "$tmp_dir/clipboard.txt"
grep -q "^14$" "$tmp_dir/clipboard.txt"
grep -q "doubled_clipboard_value" "$tmp_dir/copied-buffer.cpp"

echo "icpp integration tests passed"

qtcling_path="${ICPP_QTCLING:-/usr/local/src/cling/bin/qtcling}"

if [[ ! -x "$qtcling_path" ]]; then
    echo "Skipping qtcling integration test: qtcling not found at $qtcling_path" >&2
    exit 0
fi

cat > "$tmp_dir/default_engine.icpp" <<'EOF'
.args
.quit
EOF

ICPP_QTCLING="$qtcling_path" "$icpp_bin" < "$tmp_dir/default_engine.icpp" \
    > "$tmp_dir/default_engine.txt" 2>&1
grep -q "engine: qtcling" "$tmp_dir/default_engine.txt"

cat > "$tmp_dir/preview.ui" <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>PreviewForm</class>
 <widget class="QWidget" name="PreviewForm">
  <property name="windowTitle">
   <string>Preview Window</string>
  </property>
  <layout class="QVBoxLayout" name="verticalLayout">
   <item>
    <widget class="QLabel" name="titleLabel">
     <property name="text">
      <string>Preview OK</string>
     </property>
    </widget>
   </item>
  </layout>
 </widget>
</ui>
EOF

cat > "$tmp_dir/preview.icpp" <<'EOF'
.preview preview.ui
.widgets
.closeall
.discard
Y
.quit
EOF

(
    cd "$tmp_dir"
    QT_QPA_PLATFORM=offscreen "$icpp_bin" --engine qtcling --qtcling "$qtcling_path" \
        < preview.icpp > preview.txt 2>&1
)
grep -q "Opened .ui preview: .*preview.ui" "$tmp_dir/preview.txt"
grep -q 'QWidget "Preview Window" visible=true' "$tmp_dir/preview.txt"
grep -q 'Closed 1 widget.' "$tmp_dir/preview.txt"

cat > "$tmp_dir/widgets_all.icpp" <<'EOF'
.include <QWidget>
.x static auto hiddenWidget = new QWidget;
.x hiddenWidget->setWindowTitle("Hidden Window");
.widgets
.widgets all
.x hiddenWidget->show();
.widgets
.closeall
.quit
EOF

(
    cd "$tmp_dir"
    QT_QPA_PLATFORM=offscreen "$icpp_bin" --engine qtcling --qtcling "$qtcling_path" \
        < widgets_all.icpp > widgets_all.txt 2>&1
)
grep -q 'No top-level widgets.' "$tmp_dir/widgets_all.txt"
grep -q 'QWidget "Hidden Window" visible=false' "$tmp_dir/widgets_all.txt"
grep -q 'QWidget "Hidden Window" visible=true' "$tmp_dir/widgets_all.txt"
grep -q 'Closed 1 widget.' "$tmp_dir/widgets_all.txt"

property_editor_lib="${ICPP_PROPERTY_EDITOR_LIB:-/Users/sugita/src/qt/tools/propertyeditor/build/property_editor.dylib}"
property_editor_include="${ICPP_PROPERTY_EDITOR_INCLUDE:-/Users/sugita/src/qt/tools/propertyeditor/src}"

if [[ -f "$property_editor_lib" && -d "$property_editor_include" ]]; then
    cat > "$tmp_dir/inspect.icpp" <<'EOF'
.preview preview.ui
.inspect
.widgets
.closeall
.quit
EOF

    (
        cd "$tmp_dir"
        QT_QPA_PLATFORM=offscreen \
        ICPP_PROPERTY_EDITOR_LIB="$property_editor_lib" \
        ICPP_PROPERTY_EDITOR_INCLUDE="$property_editor_include" \
        "$icpp_bin" --engine qtcling --qtcling "$qtcling_path" < inspect.icpp > inspect.txt 2>&1
    )
    grep -q "Opened inspector." "$tmp_dir/inspect.txt"
    grep -q 'QWidget "Preview Window" visible=true' "$tmp_dir/inspect.txt"
    grep -q 'Closed 1 widget.' "$tmp_dir/inspect.txt"
    if grep -q 'QWidget "icpp Inspector" visible=true' "$tmp_dir/inspect.txt"; then
        echo ".inspect internal window should not be listed by .widgets" >&2
        cat "$tmp_dir/inspect.txt" >&2
        exit 1
    fi
else
    echo "Skipping .inspect test: property_editor not found" >&2
fi

cat > "$tmp_dir/editor.sh" <<'EOF'
#!/usr/bin/env bash
cat > "$1" <<'CPP'
#include <QString>
#include <QVariant>
#include <QDebug>

QString concat(const QVariantList &args) {
    QString result;
    for (const QVariant &arg : args) {
        result += arg.toString();
    }
    return result;
}
CPP
EOF
chmod +x "$tmp_dir/editor.sh"

cat > "$tmp_dir/qt_edit.icpp" <<'EOF'
.edit
qDebug() << concat(QVariantList{QString("a"), QString("b")});
auto x = concat(QVariantList{12, 34, "a"});
.p x
.ptype x
.widgets
.closeall
.discard
Y
.quit
EOF

ICPP_EDITOR="$tmp_dir/editor.sh" "$icpp_bin" --engine qtcling --qtcling "$qtcling_path" \
    < "$tmp_dir/qt_edit.icpp" > "$tmp_dir/qt_edit.txt" 2>&1
grep -q '"ab"' "$tmp_dir/qt_edit.txt"
grep -q '1234a' "$tmp_dir/qt_edit.txt"
grep -q 'QString' "$tmp_dir/qt_edit.txt"
grep -q 'No top-level widgets.' "$tmp_dir/qt_edit.txt"
grep -q 'Closed 0 widgets.' "$tmp_dir/qt_edit.txt"

echo "icpp qtcling edit tests passed"
