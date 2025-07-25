#include "draw_list.hpp"

namespace gfx
{
	void DrawList::append(std::string const& pass, DrawCommand const& command)
	{
		m_commands[pass].push_back(command);
	}

	std::vector<DrawCommand> DrawList::commands(std::string const& pass) const
	{
		auto const& it = m_commands.find(pass);
		if (it == m_commands.end()) {
			return {};
		}

		return it->second;
	}
} // namespace gfx
