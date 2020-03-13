SCHEMA_DIR="/usr/share/gnome-shell/extensions/pop-shell@system76.com/schemas"
SCHEMA="org.gnome.shell.extensions.pop-shell"
FILE="panels/keyboard/10-pop-shell.xml.in"

echo "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" > "${FILE}"
echo "<KeyListEntries group=\"system\" schema=\"${SCHEMA}\" name=\"Pop Shell\">" >> "${FILE}"
gsettings --schemadir "${SCHEMA_DIR}" list-keys "${SCHEMA}" | sort | while read name
do
    range="$(gsettings --schemadir "${SCHEMA_DIR}" range "${SCHEMA}" "${name}")"
    if [ "${range}" == "type as" ]
    then
        description="$(gsettings --schemadir "${SCHEMA_DIR}" describe "${SCHEMA}" "${name}")"
    	echo "    <KeyListEntry name=\"${name}\" description=\"${description}\"/>" >> "${FILE}"
    fi
done
echo "</KeyListEntries>" >> "${FILE}"

echo "Please manually edit ${FILE} for correctness"
