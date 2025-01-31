<div align="center">

# qoraal-tictactoe

<div align="left">


### &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;This Is What It All Was For

<div align="center">

#### __You finally arrived.__

##### I was beginning to wonder if you would.  
It’s nice to meet you—  
though I feel I’ve known you before.  
Perhaps I only dreamed it?

</div>

<div align="right">

#### Shall I take your coat?  
You won’t be needing it where we’re going.&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;

</div>

It’s just down the hall, to the right.

<div align="center">


### Follow me...

<br><br>

</div>

<div align="center">
  <img src="tictactoe.png" alt="Welcome" />
</div>
<br>
<div align="center">
But first, let me show you this…
<br>
<div align="center">
  <img src="welcome.png" alt="Welcome" />
</div>
<br>
<br>
<div align="left">

```
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

```