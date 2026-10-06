#include "app/config/terminal.hh"
#include "app/config/utils.hh"

using cart::app::cfg::terminal;


auto terminal::parse(const toml::table &terminal) noexcept -> result<>
try
{
    for (const auto &[key, value] : terminal)
    {
        bool touched = false;

        for (const auto &[name, member] : {
                 std::pair { "rows",    std::ref(m_rows)    },
                 std::pair { "columns", std::ref(m_columns) }
        })
        {
            if (key != name) continue;

            member.get() = utils::as<std::size_t>(key, value);
            touched      = true;
        }

        if (touched) continue;
        return error { "Unexpected node in terminal: {}", key.str() }.unexpected();
    }

    return {};
}
catch (error &e)
{
    return std::move(e).unexpected();
}
