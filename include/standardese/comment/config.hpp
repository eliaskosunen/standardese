// Copyright (C) 2016-2019 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef STANDARDESE_COMMENT_CONFIG_HPP_INCLUDED
#define STANDARDESE_COMMENT_CONFIG_HPP_INCLUDED

#include <array>
#include <string>

#include <standardese/comment/commands.hpp>

namespace standardese
{
namespace comment
{
    /// The configuration of related to the comment syntax.
    class config
    {
    public:
        /// \returns The default command character.
        static char default_command_character() noexcept
        {
            return '\\';
        }

        /// \returns The default name for the given command or section.
        /// \group default_command_name
        static const char* default_command_name(command_type cmd) noexcept;

        /// \group default_command_name
        static const char* default_command_name(section_type cmd) noexcept;

        /// \group default_command_name
        static const char* default_command_name(inline_type cmd) noexcept;

        /// \returns The default name of a [standardese::markup::inline_section]().
        static const char* default_inline_section_name(section_type section) noexcept;

        /// \returns The default name of a [standardese::markup::list_section]().
        static const char* default_list_section_name(section_type section) noexcept;

        /// \effects Creates it giving the command character,
        /// that is, the character that introduces a command.
        explicit config(char command_character = default_command_character());

        /// \effects Sets the name for the given command or section.
        /// \group set_command_name
        void set_command_name(command_type cmd, std::string name);

        /// \group set_command_name
        void set_command_name(section_type cmd, std::string name);

        /// \group set_command_name
        void set_command_name(inline_type cmd, std::string name);

        /// \returns The command character.
        char command_character() const noexcept
        {
            return command_character_;
        }

        /// \returns The name for the given command or section.
        /// \group command_name
        const char* command_name(command_type cmd) const noexcept;

        /// \group command_name
        const char* command_name(section_type cmd) const noexcept;

        /// \group command_name
        const char* command_name(inline_type cmd) const noexcept;

        /// \returns The command or section corresponding to the given command string,
        /// or an invalid value, if it doesn't belong to anything.
        /// The command string does not contain the leading command character.
        unsigned try_lookup(const char* name) const noexcept;

        /// \returns The name of a [standardese::markup::inline_section]().
        const char* inline_section_name(section_type section) const noexcept;

        /// \returns The name of a [standardese::markup::list_section]().
        const char* list_section_name(section_type section) const noexcept;

    private:
        std::array<std::string, unsigned(inline_type::count)>  command_names_;
        std::array<std::string, unsigned(section_type::count)> inline_sections_;
        std::array<std::string, unsigned(section_type::count)> list_sections_;
        char                                                   command_character_;
    };
} // namespace comment
} // namespace standardese

#endif // STANDARDESE_COMMENT_CONFIG_HPP_INCLUDED
