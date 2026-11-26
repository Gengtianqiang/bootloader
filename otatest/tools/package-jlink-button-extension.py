from pathlib import Path
import json
import sys
import zipfile
from xml.sax.saxutils import escape


source = Path(__file__).parent / "vscode-jlink-download"
output = Path(sys.argv[1])
manifest = json.loads((source / "package.json").read_text(encoding="utf-8"))
code = source / "extension.js"
output.parent.mkdir(parents=True, exist_ok=True)

package_types = """<?xml version="1.0" encoding="utf-8"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension=".json" ContentType="application/json"/>
  <Default Extension=".js" ContentType="application/javascript"/>
  <Default Extension=".vsixmanifest" ContentType="text/xml"/>
</Types>"""

vsix_manifest = f"""<?xml version="1.0" encoding="utf-8"?>
<PackageManifest Version="2.0.0" xmlns="http://schemas.microsoft.com/developer/vsx-schema/2011">
  <Metadata>
    <Identity Language="en-US" Id="{escape(manifest['name'])}" Version="{escape(manifest['version'])}" Publisher="{escape(manifest['publisher'])}"/>
    <DisplayName>{escape(manifest['displayName'])}</DisplayName>
    <Description xml:space="preserve">{escape(manifest['description'])}</Description>
    <Properties>
      <Property Id="Microsoft.VisualStudio.Code.Engine" Value="{escape(manifest['engines']['vscode'])}"/>
      <Property Id="Microsoft.VisualStudio.Code.ExtensionKind" Value="workspace"/>
    </Properties>
  </Metadata>
  <Installation><InstallationTarget Id="Microsoft.VisualStudio.Code"/></Installation>
  <Dependencies/>
  <Assets><Asset Type="Microsoft.VisualStudio.Code.Manifest" Path="extension/package.json" Addressable="true"/></Assets>
</PackageManifest>"""

with zipfile.ZipFile(output, "w", zipfile.ZIP_DEFLATED) as package:
    package.writestr("[Content_Types].xml", package_types)
    package.writestr("extension.vsixmanifest", vsix_manifest)
    package.writestr("extension/package.json", json.dumps(manifest, ensure_ascii=False, indent=2))
    package.write(code, "extension/extension.js")

print(output)
