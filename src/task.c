/* task.c: Task manager
 * Copyright (c) 2024 Nathan Misner
 *
 * This file is part of OpenMadoola.
 *
 * OpenMadoola is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * OpenMadoola is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenMadoola. If not, see <https://www.gnu.org/licenses/>.
 */
#include "constants.h"
#include "platform.h"
#include "task.h"

typedef struct Task Task;

struct Task {
    Uint8 used;
    void (*init)(void);
    void (*update)(void);
    Uint8 initialized;
    int timer;
    Task *next;
    Task *parent;
};
static Task taskPool[64] = { 0 };
static Task *currentTask = NULL;

static void Task_Free(Task *task) {
    while (task->next) {
        Task *temp = task->next->next;
        task->next->used = 0;
        task->next = temp;
    }
    task->used = 0;
}

static void Task_FreeAll(void) {
    while (currentTask) {
        Task *temp = currentTask->parent;
        Task_Free(currentTask);
        currentTask = temp;
    }
}

static Task *Task_Create(void (*init)(void), void (*update)(void), int timer) {
    Task *task = NULL;
    for (int i = 0; i < ARRAY_LEN(taskPool); i++) {
        if (!taskPool[i].used) {
            task = &taskPool[i];
            break;
        }
    }
    if (!task) {
        Platform_ShowError("Task pool exhausted");
        return NULL;
    }

    task->used = 1;
    task->init = init;
    task->update = update;
    task->initialized = 0;
    task->timer = timer;
    task->next = NULL;
    task->parent = NULL;
    return task;
}

void Task_SetRootTimed(void (*init)(void), void (*update)(void), int timer) {
    Task_FreeAll();
    currentTask = Task_Create(init, update, timer);
}

void Task_SetRoot(void (*init)(void), void (*update)(void)) {
    Task_SetRootTimed(init, update, 0);
}

void Task_AddNextTimed(void (*init)(void), void (*update)(void), int timer) {
    Task *task = currentTask;
    while (task->next) {
        task = task->next;
    }
    task->next = Task_Create(init, update, timer);
    task->next->parent = task->parent;
}

void Task_AddNext(void (*init)(void), void (*update)(void)) {
    Task_AddNextTimed(init, update, 0);
}

void Task_AddChildTimed(void (*init)(void), void (*update)(void), int timer) {
    Task *temp = currentTask;
    currentTask = Task_Create(init, update, timer);
    currentTask->parent = temp;
    if (currentTask->init) {
        currentTask->init();
    }
}

void Task_AddChild(void (*init)(void), void (*update)(void)) {
    Task_AddChildTimed(init, update, 0);
}

void Task_Next(void) {
    if (currentTask->next) {
        Task *temp = currentTask;
        currentTask = currentTask->next;
        temp->used = 0;
    }
    else if (currentTask->parent) {
        Task *temp = currentTask;
        currentTask = currentTask->parent;
        temp->used = 0;
    }
    else {
        Platform_ShowError("No task queued!");
    }
}

void Task_Parent(void) {
    Task *temp = currentTask->parent;
    Task_Free(currentTask);
    currentTask = currentTask->parent;
}

void Task_Run(void) {
    // this is to allow tasks to spawn child tasks or switch tasks from inside
    // their init functions
    Task *temp;
    do {
        temp = currentTask;
        if (!currentTask->initialized && currentTask->init) {
            currentTask->initialized = 1;
            currentTask->init();
        }
    } while (temp != currentTask);

    if (currentTask->update) {
        currentTask->update();
    }

    // go up the task chain, decrementing timers as we go. if any task's timer
    // hits zero, free all of its child tasks, make it the current task, and move
    // to the next task.
    Task *task = currentTask;
    while (task) {
        if (task->timer) {
            task->timer--;
            if (task->timer == 0) {
                Task *temp = currentTask;
                Task *parent;
                while (temp != task) {
                    parent = temp->parent;
                    Task_Free(temp);
                    temp = parent;
                }
                currentTask = task;
                Task_Next();
                break;
            }
        }
        task = task->parent;
    }
}
