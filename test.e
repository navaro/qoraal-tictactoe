decl_name       "least common ancestor test"
decl_version    1

decl_variables {
}

decl_events {
    _evt_Self
}

statemachine lca_test {

    startstate s1111

    state s1 {
        enter (console_writeln, ""
                    "-- > enter s1 < --\r\n"
                    "------------------")
        exit (console_writeln, ""
                    "-- > exit s1 < --\r\n"
                    "------------------")
    }

    super s1 {
        state s11 {
            enter (console_writeln, "enter s11")
            exit (console_writeln, "exit s11")

        }
    }

    super s11 {

        state s111 {
            enter (console_writeln, "enter s111")
            exit (console_writeln, "exit s111")
        }
    }

    super s111 {

        state s1111 {
            enter (console_writeln, "enter s1111")
            exit (console_writeln, "exit s1111")
            event (_state_start, s1112)

        }

        state s1112 {
            enter (console_writeln, "enter s1112")
            exit (console_writeln, "exit s1112")
            event (_state_start, done)

        }
    }

    state done {
        enter (console_writeln, "enter done")
        exit (console_writeln, "exit done")

    }


}
