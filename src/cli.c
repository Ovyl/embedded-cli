/*
 * MIT License
 * 
 * Copyright (c) 2019 Sean Farrelly
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * File        cli.c
 * Created by  Sean Farrelly
 * Version     1.0
 * 
 */

/*! @file cli.c
 * @brief Implementation of command-line interface.
 */
#include "cli.h"

#include <stdint.h>
#include <string.h>

rx_data_t rx_data;

const char cli_unrecog[] = "CLI Error: Command not recognized\r\n";

/*!
 * @brief This internal API prints a message to the user on the CLI.
 */
static void cli_print(cli_t *cli, const char *msg);

/*!
 * @brief This API initialises the command-line interface.
 */
cli_status_t cli_init(cli_t *cli, uint8_t *rx_buf_ptr, uint16_t rx_buf_size)
{
    /* Reset buffer */
    //memset(rx_data.buf_ptr, 0, rx_data.buf_size);
    rx_data.current_buf_length = 0;
    rx_data.is_ready = false;
    rx_data.buf_ptr = rx_buf_ptr;
    rx_data.buf_size = rx_buf_size;
    return CLI_OK;
}

/*!
 * @brief This API deinitialises the command-line interface.
 */
cli_status_t cli_deinit(cli_t *cli)
{
    return CLI_OK;
}


/*! @brief This API must be periodically called by the user to process and execute
 *         any commands received.
 */
cli_status_t cli_process(cli_t *cli)
{
    uint8_t argc = 0;
    char *argv[MAX_ARGS];

    if(rx_data.is_ready == false)
    {
        return CLI_E_CMD_NOT_READY;
    }

    rx_data.is_ready = false;

    /* Get the first token (cmd name) */
    argv[argc] = strtok((char *)rx_data.buf_ptr, " ");

    /* Walk through the other tokens (parameters) */
    while((argv[argc] != NULL) && (argc < MAX_ARGS))
    {
        argv[++argc] = strtok(NULL, " ");
    }
    
    /* Search the command table for a matching command, using argv[0]
     * which is the command name. */
    for(size_t i = 0 ; i < cli->cmd_cnt ; i++)
    {
        if(strcmp(argv[0], cli->cmd_tbl[i].cmd) == 0)
        {
            /* Found a match, execute the associated function. */
            return cli->cmd_tbl[i].func(argc, argv);
        }
    }

    /* Command not found */
    rx_data.current_buf_length = 0;
    rx_data.is_ready = false;
    memset(rx_data.buf_ptr, 0, rx_data.buf_size);
    cli_print(cli, cli_unrecog);
    return CLI_E_CMD_NOT_FOUND;
}

/*!
 * @brief This API should be called from the devices interrupt handler whenever a
 *        character is received over the input stream.
 */
cli_status_t cli_put(cli_t *cli, char c)
{
    switch(c)
    {
        case CMD_TERMINATOR:
        {
            /* Terminate the msg and reset the msg ptr by subtracting the length. This should put us back at the beginning of the array  */
            *rx_data.buf_ptr = '\0';
            rx_data.buf_ptr -= rx_data.current_buf_length;
            rx_data.is_ready = true;
            rx_data.current_buf_length = 0;
            break;
        }
        default:
        {
            /* If we have never received anything, let's clear the buffer to have a fresh start */
            if(rx_data.current_buf_length == 0)
            {
                memset(rx_data.buf_ptr, 0, rx_data.buf_size);
            }
            /* Normal character received, add to buffer. */
            if(rx_data.current_buf_length < rx_data.buf_size)
            {
            	/* If backspace or delete remove character if we aren't at the beginning of the buffer */
            	if(c == 127 || c == 8){
            		if(rx_data.current_buf_length > 0){
						*rx_data.buf_ptr-- = '\0';
						rx_data.current_buf_length--;
            		}
            	}else{
                *rx_data.buf_ptr++ = c;
                rx_data.current_buf_length++;
            	}
            }
            else
            {
                return CLI_E_BUF_FULL;
            }
            break;
        }
    }
    return CLI_OK;
}

/*!
 * @brief Print a message on the command-line interface.
 */
static void cli_print(cli_t *cli, const char *msg)
{
    cli->println(msg);
}
