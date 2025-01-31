decl_name       "Welcome to My Page"
decl_version    1

statemachine html_test {

    startstate ready

    state ready {
        enter (html_ready)
        event (_html_render, html_head)

    }

    state html {
        enter (html_emit,       "<!DOCTYPE html>\r\n"
                                "<html lang=\"en\">\r\n")
        exit (html_emit,        "</html>\r\n")
    }

    super html {
        state html_head {
            enter (html_emit,   "<head>"
                                "<meta charset=\"UTF-8\">\r\n"
                                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n"
                                "<title>Simple Page</title>\r\n"
                                "<style>\r\n"
                                "    body {\r\n"
                                "        font-family: Arial, sans-serif;\r\n"
                                "        background: radial-gradient(circle, #1e1e2e 0%, #0c0917 90%);\r\n"
                                "        color: #333;\r\n"
                                "        display: flex;\r\n"
                                "        flex-direction: column;\r\n"
                                "        justify-content: center;\r\n"
                                "        align-items: center;\r\n"
                                "        height: 100vh;\r\n"
                                "        margin: 0;\r\n"
                                "    }\r\n"
                                "    h1 {\r\n"
                                "        color: #AD231F;\r\n"
                                "    }\r\n"
                                "</style>\r\n")
            exit (html_emit,    "</head>\r\n")
            event (_state_start, html_body)
        }
    }

    super html {
        state html_body {
            enter (html_emit,   "<body>"
                                "<h1>Welcome to My Page</h1>\r\n"
                                "<p>This is a simple message to brighten your day!</p>\r\n")
            exit (html_emit,    "</body>")
            event (_state_start, ready)

        }
    }


}
