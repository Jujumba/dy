from prompt_toolkit import print_formatted_text as print, prompt, ANSI

PROMPT = ANSI("\x1B[1;95m>>> \x1B[0m")

print(prompt(PROMPT))
# print()
# print()


