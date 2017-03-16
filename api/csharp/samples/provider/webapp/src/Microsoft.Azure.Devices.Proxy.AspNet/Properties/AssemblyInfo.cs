using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

// Setting ComVisible to false makes the types in this assembly not visible 
// to COM components.  If you need to access a type in this assembly from 
// COM, set the ComVisible attribute to true on that type.
[assembly: ComVisible(false)]

// The following GUID is for the ID of the typelib if this project is exposed to COM
[assembly: Guid("17E52C70-289F-4586-A38E-AC9F8F516B05")]

#if RELEASE_DELAY_SIGN
[assembly: AssemblyDelaySign(true)]
[assembly: AssemblyKeyFile("35MSSharedLib1024.snk")]
#endif
