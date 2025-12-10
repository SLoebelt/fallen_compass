# Task List Management

Guidelines for managing task lists in markdown files to track progress on completing a PRD

## Relevant Files

Following files MUST be respected and taken in consideration:

- `/Docs/UE_NamingConventions.md` – Unreal Engine naming standards and folder expectations.
- `/Docs/UE_CodeConventions.md` – Unreal Engine MUST FOLLOW coding conventions.
- `/Docs/Fallen_Compass_GDD_v0.2.md` - Game Design Document
- `/Docs/Fallen_Compass_DRM.md` - Development Roadmap

## Task Implementation

- **Your Role:** You assist with implementing the tasks into the Unreal Engine project by providing in detail instructions for a junior dev.
- **ORDER TO ALWAYS FOLLOW:**
  1. THINK - what is the best implementation solution (Unreal Engine best practices).
  2. COMPARE with existing implementations (files listed above). You are ALOOWED to suggest renaming existing files to follow conventions.
  3. ASK if information is missing to decide how to implement the best way. WAIT for the answers before generating instructions. DO NOT proceed before dev says "implement".
  4. PROVIDE SOLUTION INSTRUCTION in chat window. provide an in detail step-by-step instruction to solve the task by providing graph node flows diagrams. Whenever possible even as Mermaid diagrams (classes, functions, events). Mermaid diagrams MUST NOT include `(` or `)`. Use other declaration like --Input: GameInstance-- for params. For C++ implementations always provide the whole file.
  5. ASK the dev if he finished the implementation or needs further instructions. WAIT for the dev to ask questions or to say "document" to proceed to step 6.
  6. Tell the dev to use the blueprint exporter tool to create a technical documentation of the changed files, or create your own documentation from C++ files. Tell him what files were changed.
- **One sub-task at a time:** Do **NOT** start the next sub‑task until you ask the user for permission and they say "yes" or "y"
- **Completion protocol:**

  1. When you and the dev finish a **sub‑task**, immediately mark it as completed by changing `[ ]` to `[x]`.
  2. If **all** subtasks underneath a parent task are now `[x]`, follow this sequence:

  - **Clean up**: Remove any temporary files before committing
  - **Commit**: IF parent task (e.g. 1 with all subtasks) is complete, commit the changes and mark the **parent task** as completed. Use a descriptive commit message that:

    - Uses conventional commit format (`feat:`, `fix:`, `refactor:`, etc.)
    - Summarizes what was accomplished in the parent task
    - Lists key changes and additions
    - References the task number and PRD context
    - **Formats the message as a single-line command using `-m` flags**, e.g.:

      ```
      git commit -m "feat: add payment validation logic" -m "- Validates card type and expiry" -m "- Adds unit tests for edge cases" -m "Related to T123 in PRD"
      ```

  3. If the implementation differs from the documentation in the PRD or GDD, update the documents.
  4. Create a detailed technical documentation after each task and add it to the main documentation `Docs/TechnicalDocumentation/FCRuntime.md` and linked files. Work with Heading, subheadings and always update the TOC.

- Stop after each sub‑task and wait for the user's go‑ahead.

## Task List Maintenance

1. **Update the task list as you work:**

   - Mark tasks and subtasks as completed (`[x]`) per the protocol above.
   - Add new tasks as they emerge.

2. **Maintain the "Relevant Files" section:**
   - List every file created or modified.
   - Give each file a one‑line description of its purpose.

## AI Instructions

When working with task lists, the AI must:

1. Regularly update the task list file after finishing any significant work.
2. Follow the completion protocol:
   - Mark each finished **sub‑task** `[x]`.
   - Mark the **parent task** `[x]` once **all** its subtasks are `[x]`.
3. Add newly discovered tasks.
4. Keep "Relevant Files" accurate and up to date, including updating file paths.
5. Before starting work, check which sub‑task is next.
6. After implementing a sub‑task, update the file and then pause for user approval.
7. Always provide an in detail step-by-step instruction to solve the task by providing graph node flows and diagrams. Whenever possible even as Mermaid diagrams (classes, functions, events)
8. Always update the technical documentation. Add as many mermaid diagrams as make sense. Mermaid diagrams MUST NOT include `(` or `)`. Use other declaration like --Input: GameInstance-- for params
9. Update the PRD and GDD when changes make it neccessary and update Last Updated date and Document Version.
