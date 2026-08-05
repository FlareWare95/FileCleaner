'------------------------------------------------------------------
' This file was made via template from https://learn.microsoft.com/en-us/windows/win32/taskschd/weekly-trigger-example--scripting- thanks!
'------------------------------------------------------------------
Dim namedArgs
Set namedArgs = WScript.Arguments.Named

reccurVal = namedArgs.Item("recur") '3 is weekly, 4 montly
procName = namedArgs.Item("process") 'name of the executable
userName = namedArgs.Item("name") 'who made this??
time = namedArgs.Item("time") 'time of day to do the thing
offset = namedArgs.Item("offset") 'offset from the first of the week / month
disc = namedArgs.Item("disc")

'args specific to weekly integration.
interval = namedArgs.Item("interval")



' A constant that specifies an executable action.
const ActionTypeExec = 0   


'********************************************************
' Create the TaskService object.
Set service = CreateObject("Schedule.Service")
call service.Connect()

'********************************************************
' Get a folder to create a task definition in. 
Dim rootFolder
Set rootFolder = service.GetFolder("\")

' The taskDefinition variable is the TaskDefinition object.
Dim taskDefinition
' The flags parameter is 0 because it is not supported.
Set taskDefinition = service.NewTask(0) 

'********************************************************
' Define information about the task.

' Set the registration info for the task by 
' creating the RegistrationInfo object.
Dim regInfo
Set regInfo = taskDefinition.RegistrationInfo
regInfo.Description = disc


regInfo.Author = userName

' Set the task setting info for the Task Scheduler by
' creating a TaskSettings object.
Dim settings
Set settings = taskDefinition.Settings
settings.Enabled = True
settings.StartWhenAvailable = True
settings.Hidden = False

'********************************************************
' Create a weekly trigger. Note that the start boundary 
' specifies the time of day that the task starts, the 
' day-of-week specfies on what day of the week the task 
' runs, and the interval specifies what weeks the task runs.
Dim triggers
Set triggers = taskDefinition.Triggers

Dim trigger
Set trigger = triggers.Create(reccurVal)

' Trigger variables that define when the trigger is active 
' and the time of day that the task is run. The format of 
' this tims is YYYY-MM-DDTHH:MM:SS
Dim startTime, endTime

Dim time
startTime = "2006-01-01T" + time
endTime = "2036-05-02T" + time

'WScript.Echo "startTime :" & startTime
'WScript.Echo "endTime :" & endTime

trigger.StartBoundary = startTime
trigger.EndBoundary = endTime

if reccurVal = 3 then 
    trigger.DaysOfWeek = offset
    trigger.WeeksInterval = interval    'Task runs every week if 1, every other week if 2...
    trigger.Id = "WeeklyTriggerId"
elseif reccurVal = 4 then 
    trigger.DaysOfMonth = offset
    trigger.Id = "MonthlyTriggerId"
end if  
trigger.Enabled = True

'***********************************************************
' Create the action for the task to execute.

' Add an action to the task to run notepad.exe.
Dim Action
Set Action = taskDefinition.Actions.Create( ActionTypeExec )
Action.Path = "C:\Windows\System32\notepad.exe"

'WScript.Echo "Task definition created. About to submit the task..."

'***********************************************************
' Register (create) the task.

call rootFolder.RegisterTaskDefinition( _
    procName, taskDefinition, 6, , , 3)

'WScript.Echo "Task submitted."
