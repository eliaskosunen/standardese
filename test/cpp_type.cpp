// Copyright (C) 2016 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <standardese/cpp_class.hpp>
#include <standardese/cpp_enum.hpp>
#include <standardese/cpp_namespace.hpp>
#include <standardese/cpp_type.hpp>

#include <catch.hpp>

#include "test_parser.hpp"

using namespace standardese;

TEST_CASE("cpp_type_alias", "[cpp]")
{
    parser p;

    auto code = R"(
        using /* comment */ type_1 = unsigned int;
        using type_2 = char[];

        typedef int type_3;

        struct foo {};

        using type_4 = const foo *;

        namespace ns
        {
            struct foo {};

            using type_5 = foo;
        }

        using type_6 = ns::foo;

        typedef void(*type_7)(int, char);
    )";

    auto tu = parse(p, "cpp_type_alias", code);

    auto count = 0u;
    auto lambda = [&](const cpp_entity &e)
    {
        if (!dynamic_cast<const cpp_type_alias*>(&e))
            return;

        auto& t = dynamic_cast<const cpp_type_alias&>(e);
        if (t.get_name() == "type_1")
        {
            ++count;
            REQUIRE(t.get_full_name() == "type_1");
            auto& target = t.get_target();
            REQUIRE(target.get_name() == "unsigned int");
            REQUIRE(target.get_full_name() == "unsigned int");
        }
        else if (t.get_name() == "type_2")
        {
            ++count;
            REQUIRE(t.get_full_name() == "type_2");
            auto& target = t.get_target();
            REQUIRE(target.get_name() == "char[]");
            // note: whitespace because libclang inserts space there
            REQUIRE(target.get_full_name() == "char []");
        }
        else if (t.get_name() == "type_3")
        {
            ++count;
            REQUIRE(t.get_full_name() == "type_3");
            auto& target = t.get_target();
            REQUIRE(target.get_name() == "int");
            REQUIRE(target.get_full_name() == "int");
        }
        else if (t.get_name() == "type_4")
        {
            ++count;
            REQUIRE(t.get_full_name() == "type_4");
            auto& target = t.get_target();
            REQUIRE(target.get_name() == "const foo *");
            REQUIRE(target.get_full_name() == "const foo *");
        }
        else if (t.get_name() == "type_5")
        {
            ++count;
            REQUIRE(t.get_full_name() == "ns::type_5");
            auto& target = t.get_target();
            REQUIRE(target.get_name() == "foo");
            REQUIRE(target.get_full_name() == "ns::foo");
        }
        else if (t.get_name() == "type_6")
        {
            ++count;
            REQUIRE(t.get_full_name() == "type_6");
            auto& target = t.get_target();
            REQUIRE(target.get_name() == "ns::foo");
            REQUIRE(target.get_full_name() == "ns::foo");
        }
        else if (t.get_name() == "type_7")
        {
            ++count;
            REQUIRE(t.get_full_name() == "type_7");
            auto& target = t.get_target();
            REQUIRE(target.get_name() == "void(*)(int, char)");
            REQUIRE(target.get_full_name() == "void (*)(int, char)");
        }
        else
            REQUIRE(false);
    };

    for (auto& e : tu.get_file())
    {
        if (e.get_entity_type() == cpp_entity::namespace_t)
            for (auto& child : static_cast<const cpp_namespace&>(e))
                lambda(child);
        else
            lambda(e);
    }
    REQUIRE(count == 7u);
}

TEST_CASE("cpp_enum", "[cpp]")
{
    parser p;

    auto code = R"(
        enum ignore_me : int;
        enum class ignore_me_as_well;

        enum a
        {
            a_1,
            a_2,
            a_3 = 5
        };

        enum class [[deprecated]] b : int
        {
            b_1,
            b_2,
            b_3 = -1
        };
    )";

    auto tu = parse(p, "cpp_enum", code);

    auto count = 0u;
    for_each(tu.get_file(), [&](const cpp_entity &e)
    {
        auto &t = dynamic_cast<const cpp_enum&>(e);
        REQUIRE(t.get_name() == t.get_full_name());

        if (t.get_name() == "a")
        {
            REQUIRE(!t.is_scoped());
            auto& underlying = t.get_underlying_type();
            REQUIRE(underlying.get_name() == "");
            REQUIRE(underlying.get_full_name() == "unsigned int");

            auto i = 1u;
            for (auto& val : t)
            {
                auto& eval = dynamic_cast<const cpp_unsigned_enum_value&>(val);
                REQUIRE(eval.get_name() == "a_" + std::to_string(i));
                REQUIRE(eval.get_full_name() == "a_" + std::to_string(i));

                if (i == 3u)
                {
                    REQUIRE(eval.get_value() == 5);
                    REQUIRE(eval.is_explicitly_given());
                }
                else
                {
                    REQUIRE(eval.get_value() == i - 1);
                    REQUIRE(!eval.is_explicitly_given());
                }
                ++i;
            }
            REQUIRE(i == 4u);

            ++count;
        }
        else if (t.get_name() == "b")
        {
            REQUIRE(t.is_scoped());
            auto& underlying = t.get_underlying_type();
            REQUIRE(underlying.get_name() == "int");
            REQUIRE(underlying.get_full_name() == "int");

            auto i = 1u;
            for (auto& val : t)
            {
                auto& eval = dynamic_cast<const cpp_signed_enum_value&>(val);
                REQUIRE(eval.get_name() == "b_" + std::to_string(i));
                REQUIRE(eval.get_full_name() == "b::b_" + std::to_string(i));

                if (i == 3u)
                {
                    REQUIRE(eval.get_value() == -1);
                    REQUIRE(eval.is_explicitly_given());
                }
                else
                {
                    REQUIRE(eval.get_value() == i - 1);
                    REQUIRE(!eval.is_explicitly_given());
                }
                ++i;
            }
            REQUIRE(i == 4u);

            ++count;
        }
        else
            REQUIRE(false);
    });
    REQUIRE(count == 2u);
}

TEST_CASE("cpp_class", "[cpp]")
{
    parser p;

    // no need to test member functions here
    auto code = R"(
        struct ignore_me;
        class ignore_me_too;
        union ignore_me_three;

        struct [[deprecated]] foo final
        {
        public:
            struct nested_a {};
        protected:
            struct nested_b {};
        private:
            struct nested_c {};
        };

        union union_t
        {

        };

        class base_a {};
        class base_b {};
        class base_c {};

        class derived
        : public base_a, private virtual base_b, protected base_c {};
    )";

    auto tu = parse(p, "cpp_class", code);

    auto count = 0u;
    for_each(tu.get_file(), [&](const cpp_entity &e)
    {
        auto& c = dynamic_cast<const cpp_class&>(e);
        if (c.get_name() == "foo")
        {
            ++count;
            REQUIRE(c.is_final());
            REQUIRE(c.get_class_type() == cpp_struct_t);
            for (auto& member : c)
            {
                if (member.get_name() == "public")
                {
                    ++count;
                    auto& access = dynamic_cast<const cpp_access_specifier&>(member);
                    REQUIRE(access.get_access() == cpp_public);
                }
                else if (member.get_name() == "private")
                {
                    ++count;
                    auto& access = dynamic_cast<const cpp_access_specifier&>(member);
                    REQUIRE(access.get_access()  == cpp_private);
                }
                else if (member.get_name() == "protected")
                {
                    ++count;
                    auto& access = dynamic_cast<const cpp_access_specifier&>(member);
                    REQUIRE(access.get_access()  == cpp_protected);
                }
                else if (member.get_name() == "nested_a")
                {
                    ++count;
                    REQUIRE(!dynamic_cast<const cpp_class&>(member).is_final());
                    REQUIRE(member.get_full_name() == "foo::nested_a");
                    REQUIRE(dynamic_cast<const cpp_class&>(member).get_class_type() == cpp_struct_t);
                }
                else if (member.get_name() == "nested_b")
                {
                    ++count;
                    REQUIRE(!dynamic_cast<const cpp_class&>(member).is_final());
                    REQUIRE(member.get_full_name() == "foo::nested_b");
                    REQUIRE(dynamic_cast<const cpp_class&>(member).get_class_type() == cpp_struct_t);
                }
                else if (member.get_name() == "nested_c")
                {
                    ++count;
                    REQUIRE(!dynamic_cast<const cpp_class&>(member).is_final());
                    REQUIRE(member.get_full_name() == "foo::nested_c");
                    REQUIRE(dynamic_cast<const cpp_class&>(member).get_class_type() == cpp_struct_t);
                }
                else
                    REQUIRE(dynamic_cast<const cpp_class*>(&member) != nullptr);
            }
        }
        else if (c.get_name() == "union_t")
        {
            ++count;
            REQUIRE(!c.is_final());
            REQUIRE(c.get_class_type() == cpp_union_t);
        }
        else if (c.get_name() == "base_a")
        {
            ++count;
            REQUIRE(!c.is_final());
            REQUIRE(c.get_class_type() == cpp_class_t);
        }
        else if (c.get_name() == "base_b")
        {
            ++count;
            REQUIRE(!c.is_final());
            REQUIRE(c.get_class_type() == cpp_class_t);
        }
        else if (c.get_name() == "base_c")
        {
            ++count;
            REQUIRE(!c.is_final());
            REQUIRE(c.get_class_type() == cpp_class_t);
        }
        else if (c.get_name() == "derived")
        {
            ++count;
            REQUIRE(!c.is_final());
            for (auto& base : c.get_bases())
            {
                if (base.get_name() == "base_a")
                {
                    ++count;
                    REQUIRE(base.get_access() == cpp_public);
                    REQUIRE(!base.is_virtual());
                }
                else if (base.get_name() == "base_b")
                {
                    ++count;
                    REQUIRE(base.get_access() == cpp_private);
                    REQUIRE(base.is_virtual());
                }
                else if (base.get_name() == "base_c")
                {
                    ++count;
                    REQUIRE(base.get_access() == cpp_protected);
                    REQUIRE(!base.is_virtual());
                }
            }
        }
        else
            REQUIRE(false);
    });
    REQUIRE(count == 15u);
}