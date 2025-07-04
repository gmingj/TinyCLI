#!/usr/bin/env python3
"""
TinyCLI Plugin Generator

This script generates plugin skeleton code from a JSON configuration file.
"""

import os
import sys
import json
import argparse
from datetime import datetime

# Template for plugin C file
PLUGIN_C_TEMPLATE = '''/**
 * @file {plugin_name}.c
 * @brief {plugin_description}
 * @version {plugin_version}
 * @date {date}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinycli.h"
#include "context.h"
#include "command.h"
#include "plugin.h"
#include "utils.h"

{command_handlers}

/**
 * @brief Plugin initialization function
 * @param ctx TinyCLI context
 * @return Error code
 */
int tinycli_plugin_init(tinycli_context_t *ctx)
{{
    int ret;

    printf("Initializing {plugin_name} plugin (v{plugin_version})\\n");

{register_commands}

    return TINYCLI_SUCCESS;
}}

/**
 * @brief Plugin cleanup function
 * @param ctx TinyCLI context
 */
void tinycli_plugin_cleanup(tinycli_context_t *ctx)
{{
    printf("Cleaning up {plugin_name} plugin\\n");
}}
'''

# Template for command handler
COMMAND_HANDLER_TEMPLATE = '''/**
 * @brief Handler for {command_name} command
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @param ctx TinyCLI context
 * @return Error code
 */
static int {handler_name}(int argc, char **argv, tinycli_context_t *ctx)
{{
    // TODO: Implement {command_name} command
    tinycli_printf(ctx, "Command '{command_name}' not implemented yet\\n");
    return TINYCLI_SUCCESS;
}}
'''

# Template for command registration
COMMAND_REGISTER_TEMPLATE = '''    /* Register {command_name} command */
    ret = tinycli_register_command(ctx, "{command_name}", "{command_help}", {handler_name}, NULL);
    if (ret != TINYCLI_SUCCESS) {{
        return ret;
    }}
'''

def generate_plugin(json_file, output_dir):
    """Generate plugin code from JSON configuration."""
    try:
        with open(json_file, 'r') as f:
            config = json.load(f)
    except (IOError, json.JSONDecodeError) as e:
        print(f"Error reading JSON file: {e}", file=sys.stderr)
        return 1

    # Extract plugin information
    plugin_name = config.get('name')
    plugin_description = config.get('description', 'TinyCLI plugin')
    plugin_version = config.get('version', '1.0.0')
    commands = config.get('commands', [])

    if not plugin_name:
        print("Error: Plugin name is required", file=sys.stderr)
        return 1

    # Generate command handlers
    command_handlers = []
    for cmd in commands:
        command_name = cmd.get('name')
        handler_name = cmd.get('handler')
        if not command_name or not handler_name:
            continue
        
        command_handlers.append(COMMAND_HANDLER_TEMPLATE.format(
            command_name=command_name,
            handler_name=handler_name
        ))

    # Generate command registrations
    register_commands = []
    for cmd in commands:
        command_name = cmd.get('name')
        command_help = cmd.get('help', '')
        handler_name = cmd.get('handler')
        if not command_name or not handler_name:
            continue
        
        register_commands.append(COMMAND_REGISTER_TEMPLATE.format(
            command_name=command_name,
            command_help=command_help,
            handler_name=handler_name
        ))

    # Generate plugin C file
    plugin_c = PLUGIN_C_TEMPLATE.format(
        plugin_name=plugin_name,
        plugin_description=plugin_description,
        plugin_version=plugin_version,
        date=datetime.now().strftime("%Y-%m-%d"),
        command_handlers='\n'.join(command_handlers),
        register_commands='\n'.join(register_commands)
    )

    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)

    # Write plugin C file
    output_file = os.path.join(output_dir, f"{plugin_name}.c")
    try:
        with open(output_file, 'w') as f:
            f.write(plugin_c)
    except IOError as e:
        print(f"Error writing output file: {e}", file=sys.stderr)
        return 1

    print(f"Generated plugin code: {output_file}")
    return 0

def main():
    """Main function."""
    parser = argparse.ArgumentParser(description='TinyCLI Plugin Generator')
    parser.add_argument('json_file', help='JSON configuration file')
    parser.add_argument('-o', '--output-dir', default='.', help='Output directory')
    args = parser.parse_args()

    return generate_plugin(args.json_file, args.output_dir)

if __name__ == '__main__':
    sys.exit(main()) 