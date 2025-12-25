add_requires("luisa-compute", {configs = {
    cuda = true
}})

set_runtimes("MT")

add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
-- xmake project -k compile_commands .vscode
set_languages("c++20")

target("main")
    set_encodings("utf-8")
    set_kind("binary")
    set_default()

    add_packages("luisa-compute")
    add_files("src/main.cpp")

    -- $OutputEncoding = [Console]::InputEncoding = [Console]::OutputEncoding = New-Object System.Text.UTF8Encoding($false)
    -- xmake f -p windows -a x64 -m release --toolchain=clang-cl

    on_config(function (target)
        -- Use target:targetdir() path
        -- Context context{argv[0]};
        os.vcp(path.join(target:pkg("luisa-compute"):installdir(), "bin/*"), target:targetdir())
    end)
target_end()
