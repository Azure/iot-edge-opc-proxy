using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

// General Information about an assembly is controlled through the following 
// set of attributes. Change these attribute values to modify the information
// associated with an assembly.
[assembly: AssemblyTitle("Opc.Ua.Proxy")]
[assembly: AssemblyDescription("")]
[assembly: AssemblyConfiguration("")]
[assembly: DefaultDllImportSearchPaths(DllImportSearchPath.SafeDirectories)] 

// Setting ComVisible to false makes the types in this assembly not visible 
// to COM components.  If you need to access a type in this assembly from 
// COM, set the ComVisible attribute to true on that type.
[assembly: ComVisible(false)]

// The following GUID is for the ID of the typelib if this project is exposed to COM
[assembly: Guid("664C8EA4-DD77-4F35-AFC5-27F8A6E98234")]

[assembly: AssemblyVersion("0.1.5.0")]

#if RELEASE_DELAY_SIGN
[assembly: AssemblyDelaySign(true)]
[assembly: AssemblyKeyFile("35MSSharedLib1024.snk")]
#endif

// Version information for an assembly follows semantic versioning 1.0.0 (because
// NuGet didn't support semver 2.0.0 before VS 2015). See semver.org for details.
[assembly: AssemblyInformationalVersion("0.1.5")]
