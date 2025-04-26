.section .text
.global task_exit_trampoline
.extern task_exit

task_exit_trampoline:
    b task_exit
