add_requires("luisa-compute", {configs = {
    cuda = true
}})

set_runtimes("MT")

set_languages("c++20")

target("main")
    set_kind("binary")
    add_files("src/main.cpp")
    add_packages("luisa-compute")
    set_default()

    -- $OutputEncoding = [Console]::InputEncoding = [Console]::OutputEncoding = New-Object System.Text.UTF8Encoding($false)
    -- xmake f -p windows -a x64 -m release --toolchain=clang-cl

    on_config(function (target)
        -- Use target:targetdir() path
        -- Context context{argv[0]};
        os.vcp(path.join(target:pkg("luisa-compute"):installdir(), "bin/*"), target:targetdir())
    end)
