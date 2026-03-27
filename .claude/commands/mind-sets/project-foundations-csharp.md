---
description: "Project Foundations - C#/.NET implementation"
---

# Project Foundations — C#/.NET

$include: ./project-foundations.md

---

## 1. Zero-Tolerance Warnings

```xml
<!-- Directory.Build.props -->
<PropertyGroup>
  <TreatWarningsAsErrors>true</TreatWarningsAsErrors>
  <Nullable>enable</Nullable>
  <AnalysisLevel>latest-recommended</AnalysisLevel>
</PropertyGroup>
```

Nullable reference warnings force you to handle null explicitly. Every unused variable, deprecated API call, and type mismatch is caught at compile time.

---

## 2. Central Dependency Management

```xml
<!-- Directory.Packages.props (solution root) -->
<Project>
  <PropertyGroup>
    <ManagePackageVersionsCentrally>true</ManagePackageVersionsCentrally>
    <CentralPackageTransitivePinningEnabled>true</CentralPackageTransitivePinningEnabled>
  </PropertyGroup>
  <ItemGroup>
    <PackageVersion Include="Microsoft.Extensions.Logging.Abstractions" Version="9.0.0" />
    <PackageVersion Include="NSubstitute" Version="5.3.0" />
    <!-- ALL versions here, nowhere else -->
  </ItemGroup>
</Project>
```

In `.csproj` files, reference packages WITHOUT version:
```xml
<PackageReference Include="Microsoft.Extensions.Logging.Abstractions" />
```

Upgrading a package = single-line change. Transitive pinning prevents hidden version mismatches.

---

## 3. Versioning Strategy

```
version.build          → single file, single number (e.g., "42")
Directory.Build.props  → reads it: <Version>1.0.$(_BuildNumber)</Version>
```

```xml
<!-- Directory.Build.props -->
<PropertyGroup>
  <_BuildNumber>$([System.IO.File]::ReadAllText('$(MSBuildThisFileDirectory)version.build').Trim())</_BuildNumber>
  <Version>1.0.$(_BuildNumber)</Version>
</PropertyGroup>
```

The application reports its version at startup and via a health/version endpoint. Every DLL carries the version. Bump `version.build` before every build.

---

## 4. Structured Error Handling

```csharp
public record AppError(AppErrorCode Code, string Message,
    IReadOnlyList<string>? Candidates = null, string? Suggestion = null);

public enum AppErrorCode
{
    NotFound, InvalidParams, Unauthorized,
    Timeout, NotSupported, InternalError
}

// Consistent formatting
public static class ErrorHelper
{
    public static string Format(AppError error) => ...;
}
```

Every error has a typed code — consumers react programmatically. `Suggestion` helps users fix the problem. A shared `ErrorHelper` ensures consistent formatting across all endpoints.

---

## 5. Test Configuration (Fast/Slow Split)

```xml
<!-- test.runsettings -->
<RunSettings>
  <RunConfiguration>
    <TestCaseFilter>Category!=Manual</TestCaseFilter>
  </RunConfiguration>
</RunSettings>
```

```csharp
[Trait("Category", "Manual")]  // slow/integration tests
public class EfCoreIntegrationTests { ... }
```

- `dotnet test -s test.runsettings` → fast tests only (seconds)
- `dotnet test --filter "Category=Manual"` → slow tests on demand

---

## 6. Immutable DTOs

```csharp
public record UserDto(string Name, string Email, DateTime CreatedAt);
public record ApiResponse<T>(T Data, int TotalCount);

// Derived copies with 'with'
var updated = user with { Email = "new@email.com" };
```

Use `record` for all DTOs. Value equality by default. `with` expressions for derived copies. Collections use `IReadOnlyList<T>`, `IReadOnlyDictionary<TK,TV>`.

---

## 7. Dependency Injection with Proper Registration

```csharp
// Each library exposes its own extension method
public static class ServiceCollectionExtensions
{
    public static IServiceCollection AddAppCore(this IServiceCollection services)
    {
        services.AddSingleton<IMyService, MyService>();
        services.AddScoped<IRepository, Repository>();
        return services;
    }
}

// Program.cs — clean composition root
builder.Services.AddAppCore();
builder.Services.AddAppInfrastructure(builder.Configuration);
builder.Services.AddAppPlugins(options.PluginPath);
```

Program.cs is readable in 10 seconds. Each `Add*()` method is self-contained.

---

## 8. Centralized Configuration with Validation

```json
// appsettings.json — ALL values explicit
{
  "App": {
    "DataPath": "/app/data",
    "MaxRetries": 3,
    "TimeoutSeconds": 30
  }
}
```

```csharp
// Validation at startup — fail fast
services.AddOptions<AppOptions>()
    .Bind(configuration.GetSection("App"))
    .ValidateDataAnnotations()
    .ValidateOnStart();

// Or PostConfigure for custom validation
services.PostConfigure<AppOptions>(options =>
{
    if (string.IsNullOrEmpty(options.DataPath))
        throw new InvalidOperationException("App:DataPath is required");
});
```

Consumed via `IOptions<T>` / `IOptionsMonitor<T>` through DI. Never read config files directly.

---

## 9. Structured File Logging

```csharp
// Program.cs — Serilog as implementation
builder.Host.UseSerilog((context, config) =>
    config.ReadFrom.Configuration(context.Configuration));
```

```json
// appsettings.json — configured, not hardcoded
{
  "Serilog": {
    "MinimumLevel": { "Default": "Information" },
    "WriteTo": [{ "Name": "File", "Args": { "path": "logs/app-.log", "rollingInterval": "Day" } }]
  }
}
```

```csharp
// Libraries — ONLY Microsoft.Extensions.Logging abstractions
public class MyService
{
    private readonly ILogger<MyService> _logger;
    public MyService(ILogger<MyService> logger) => _logger = logger;
}
```

Only the host project references Serilog. Libraries use `ILogger<T>`. In tests, use `NullLogger<T>.Instance`.

---

## 10. Zero-Dependency Core Module

```
MyApp.Core → zero NuGet dependencies, only Microsoft.Extensions.*.Abstractions
```

Core contains: interfaces, DTOs (records), enums, constants. No implementations. Every other project references Core. Core references nothing.

---

## 11. Interface-First Design

```csharp
public interface IUserService
{
    Task<UserDto?> GetByIdAsync(int id, CancellationToken ct = default);
}

public interface IRepository<T> where T : class
{
    Task<T?> FindAsync(int id, CancellationToken ct = default);
}
```

Every significant service has an interface in Core. Implementations live in their own projects. Testing uses NSubstitute/Moq against interfaces.

---

## 12. Internal/Private by Default

```csharp
internal sealed class UserService : IUserService { ... }
internal static class StringExtensions { ... }
internal sealed class PluginLoadContext : AssemblyLoadContext { ... }
```

Only interfaces and DTOs are `public`. Implementations are `internal sealed`. Grant test access with:

```csharp
// In .csproj or AssemblyInfo.cs
[assembly: InternalsVisibleTo("MyApp.Tests")]
```

---

## 13. Test Module per Production Module

```
MyApp.Core       → MyApp.Core.Tests
MyApp.Server     → MyApp.Server.Tests
MyApp.Plugin.Foo → MyApp.Plugin.Foo.Tests
```

Each test project tests exactly one production project. A failure tells you which module is broken.

---

## 14. Convention Over Configuration

Examples:
- Plugin discovery: drop DLL + deps.json in `plugins/<name>/` → auto-discovered
- JSON config files in a known directory define behavior — zero C# code to add a variant
- `IPluginFactory.CanHandle(path)` lets components self-select

---

## 15. Deterministic Build Output

```xml
<!-- Directory.Build.props -->
<PropertyGroup Condition="$([MSBuild]::IsOSPlatform('Linux'))">
  <BaseOutputPath>$(HOME)/app-build/bin/$(MSBuildProjectName)/</BaseOutputPath>
  <BaseIntermediateOutputPath>$(HOME)/app-build/obj/$(MSBuildProjectName)/</BaseIntermediateOutputPath>
</PropertyGroup>
```

Build output always in a known location. No stale `bin/obj` confusion between Windows and Linux.

---

## 16. Black Box Composition

```csharp
// Public interface in Core — the contract
public interface IOrderProcessor
{
    Task<OrderResult> ProcessAsync(OrderRequest request, CancellationToken ct = default);
}

// Internal implementation — hidden behind the interface
internal sealed class OrderProcessor : IOrderProcessor { ... }
```

Each black box exposes `public interface` + `public record` DTOs from Core. Implementation is `internal sealed`. Assembly-level isolation via separate projects. Test each box in isolation via its interface.

---

## Verification Commands

```bash
# Check warnings-as-errors is set
grep -r "TreatWarningsAsErrors" Directory.Build.props
# Check central package management
grep -r "ManagePackageVersionsCentrally" Directory.Packages.props
# Check no hardcoded versions in csproj
grep -r "Version=" --include="*.csproj" | grep -v "PackageReference"
# Build clean
dotnet build --no-incremental -warnaserror
# Run fast tests
dotnet test -s test.runsettings
```
