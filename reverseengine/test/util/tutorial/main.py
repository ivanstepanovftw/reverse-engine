#!/usr/bin/env python3
import sys
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtWidgets import *
import gettext
import random
import datetime


_ = gettext.gettext

text = {
    "step1": """Welcome to the Cheat Engine Tutorial (v3.3)

This tutorial will teach you the basics of cheating in video games. It will also show you foundational aspects of using Cheat Engine (or CE for short). Follow the steps below to get started.

1: Open Cheat Engine if it currently isn't running.
2: Click on the "Open Process" icon (it's the top-left icon with the computer on it, below "File".).
3: With the Process List window now open, look for this tutorial's process in the list. It will look something like "00001F98-Tutorial-x86_64.exe" or "0000047C-Tutorial-i386.exe". (The first 8 numbers/letters will probably be different.)
4: Once you've found the process, click on it to select it, then click the "Open" button. (Don't worry about all the other buttons right now. You can learn about them later if you're interested.)

Congratulations! If you did everything correctly, the process window should be gone with Cheat Engine now attached to the tutorial (you will see the process name towards the top-center of CE).

Click the "Next" button below to continue, or fill in the password and click the "OK" button to proceed to that step.)

If you're having problems, simply head over to forum.cheatengine.org, then click on "Tutorials" to view beginner-friendly guides!""",
    "step2": """Step 2: Exact Value scanning (PW=090453)
Now that you have opened the tutorial with Cheat Engine let's get on with the next step.

You can see at the bottom of this window is the text Health: xxx
Each time you click 'Hit me'  your health gets decreased.

To get to the next step you have to find this value and change it to 1000

To find the value there are different ways, but I'll tell you about the easiest, 'Exact Value':
First make sure value type is set to at least 2-bytes or 4-bytes. 1-byte will also work, but you'll run into an easy to fix problem when you've found the address and want to change it. The 8-byte may perhaps works if the bytes after the address are 0, but I wouldn't take the bet.
Single, double, and the other scans just don't work, because they store the value in a different way.

When the value type is set correctly, make sure the scantype is set to 'Exact Value'
Then fill in the number your health is in the value box. And click 'First Scan'
After a while (if you have a extremely slow pc) the scan is done and the results are shown in the list on the left

If you find more than 1 address and you don't know for sure which address it is, click 'Hit me', fill in the new health value into the value box, and click 'Next Scan'
repeat this until you're sure you've found it. (that includes that there's only 1 address in the list.....)

Now double click the address in the list on the left. This makes the address pop-up in the list at the bottom, showing you the current value.
Double click the value, (or select it and press enter), and change the value to 1000.

If everything went ok the next button should become enabled, and you're ready for the next step.


Note:
If you did anything wrong while scanning, click "New Scan" and repeat the scanning again.
Also, try playing around with the value and click 'hit me'""",
    "step3": """Step 3: Unknown initial value (PW=419482)
Ok, seeing that you've figured out how to find a value using exact value let's move on to the next step.

First things first though. Since you are doing a new scan, you have to click on New Scan first, to start a new scan. (You may think this is straighforward, but you'd be surprised how many people get stuck on that step) I won't be explaining this step again, so keep this in mind
Now that you've started a new scan, let's continue

In the previous test we knew the initial value so we could do a exact value, but now we have a status bar where we don't know the starting value.
We only know that the value is between 0 and 500. And each time you click 'hit me' you lose some health. The amount you lose each time is shown above the status bar.

Again there are several different ways to find the value. (like doing a decreased value by... scan), but I'll only explain the easiest. "Unknown initial value", and decreased value.
Because you don't know the value it is right now, a exact value wont do any good, so choose as scantype 'Unknown initial value', again, the value type is 4-bytes. (most windows apps use 4-bytes)click first scan and wait till it's done.

When it is done click 'hit me'. You'll lose some of your health. (the amount you lost shows for a few seconds and then disappears, but you don't need that)
Now go to Cheat Engine, and choose 'Decreased Value' and click 'Next Scan'
When that scan is done, click hit me again, and repeat the above till you only find a few. 

We know the value is between 0 and 500, so pick the one that is most likely the address we need, and add it to the list.
Now change the health to 5000, to proceed to the next step.""",
    "step4": """Step 4: Floating points (PW=890124)
In the previous tutorial we used bytes to scan, but some games store information in so called 'floating point' notations. 
(probably to prevent simple memory scanners from finding it the easy way)
a floating point is a value with some digits behind the point. (like 5.12 or 11321.1)

Below you see your health and ammo. Both are stored as Floating point notations, but health is stored as a float and ammo is stored as a double.
Click on hit me to lose some health, and on shoot to decrease your ammo with 0.5
 
You have to set BOTH values to 5000 or higher to proceed.

Exact value scan will work fine here, but you may want to experiment with other types too.














Hint: It is recommended to disable "Fast Scan" for type double""",
    "step5": """Step 5: Code finder (PW=888899)
Sometimes the location something is stored at changes when you restart the game, or even while you're playing.. In that case you can use 2 things to still make a table that works.
In this step I'll try to describe how to use the Code Finder function.

The value down here will be at a different location each time you start the tutorial, so a normal entry in the address list wouldn't work.
First try to find the address. (you've got to this point so I assume you know how to)
When you've found the address, right-click the address in Cheat Engine and choose "Find out what writes to this address". A window will pop up with an empty list.
Then click on the Change value button in this tutorial, and go back to Cheat Engine. If everything went right there should be an address with assembler code there now.
Click it and choose the replace option to replace it with code that does nothing. That will also add the code address to the code list in the advanced options window. (Which gets saved if you save your table)

Click on stop, so the game will start running normal again, and close to close the window.
Now, click on Change value, and if everything went right the Next button should become enabled.

Note: When you're freezing the address with a high enough speed it may happen that next becomes visible anyhow""",
    "step6": """Step 6: Pointers: (PW=098712)
In the previous step I explained how to use the Code finder to handle changing locations. But that method alone makes it difficult to find the address to set the values you want.
That's why there are pointers:

At the bottom you'll find 2 buttons. One will change the value, and the other changes the value AND the location of the value.
For this step you don't really need to know assembler, but it helps a lot if you do.

First find the address of the value. When you've found it use the function to find out what accesses this address.
Change the value again, and a item will show in the list. Double click that item. (or select and click on more info) and a new window will open with detailed information on what happened when the instruction ran.
If the assembler instruction doesn't have anything between a '[' and ']' then use another item in the list.
If it does it will say what it think will be the value of the pointer you need.
Go back to the main cheat engine window (you can keep this extra info window open if you want, but if you close it, remember what is between the [ and ] ) and do a 4 byte scan in hexadecimal for the value the extra info told you.
When done scanning it may return 1 or a few hundred addresses. Most of the time the address you need will be the smallest one. Now click on manually add and select the pointer checkbox.

The window will change and allow you to type in the address of a pointer and a offset.
Fill in as address the address you just found.
If the assembler instruction has a calculation (e.g: [esi+12]) at the end then type the value in that's at the end. else leave it 0. If it was a more complicated instruction look at the calculation.

example of a more complicated instruction:
[EAX*2+EDX+00000310] eax=4C and edx=00801234.
In this case EDX would be the value the pointer has, and EAX*2+00000310 the offset, so the offset you'd fill in would be 2*4C+00000310=3A8.  (this is all in hex, use calc.exe from windows in scientific mode to calculate)

Back to the tutorial, click OK and the address will be added, If all went right the address will show P->xxxxxxx, with xxxxxxx being the address of the value you found. If thats not right, you've done something wrong.
Now, change the value using the pointer you added in 5000 and freeze it. Then click Change pointer, and if all went 
right the next button will become visible.


extra:
And you could also use the pointer scanner to find the pointer to this address""",
    "step7": """Step 7: Code Injection: (PW=013370)
Code injection is a technique where you inject a piece of code into the target process, and then reroute the execution of code to go through your own written code.

In this tutorial you'll have a health value and a button that will decrease your health by 1 each time you click it.
Your task is to use code injection to make the button increase your health by 2 each time it is clicked

Start with finding the address and then find what writes to it.
then when you've found the code that decreases it browse to that address in the disassembler, and open the auto assembler window (ctrl+a)
There click on template and then code injection, and give it the address that decreases health (If it isn't already filled in correctly)
That will generate a basic auto assembler injection framework you can use for your code.

Notice the alloc, that will allocate a block of memory for your code cave, in the past, in the pre windows 2000 systems, people had to find code caves in the memory(regions of memory unused by the game), but that's luckily a thing of the past since windows 2000, and will these days cause errors when trying to be used, due to SP2 of XP and the NX bit of new CPU's

Also notice the line newmem: and originalcode: and the text "Place your code here"
As you guessed it, write your code here that will increase the  health with 2.
An usefull assembler instruction in this case is the "ADD instruction"
here are a few examples:
"ADD [00901234],9" to increase the address at 00901234 with 9
"ADD [ESP+4],9" to increase the address pointed to by ESP+4 with 9
In this case, you'll have to use the same thing between the brackets as the original code has that decreases your health

Notice:
It is recommended to delete the line that decreases your health from the original code section, else you'll have to increase your health with 3 (you increase with 3, the original code decreases with 1, so the end result is increase with 2), which might become confusing. But it's all up to you and your programming.

Notice 2:
In some games the original code can exist out of multiple instructions, and sometimes, not always, it might happen that a code at another place jumps into your jump instruction end will then cause unknown behavior. If that happens, you should usually look near that instruction and see the jumps and fix it, or perhaps even choose to use a different address to do the code injection from. As long as you're able to figure out the address to change from inside your injected code.""",
    "step8": """Step 8: Multilevel pointers: (PW=525927)
This step will explain how to use multi-level pointers.
In step 6 you had a simple level-1 pointer, with the first address found already being the real base address.
This step however is a level-4 pointer. It has a pointer to a pointer to a pointer to a pointer to a pointer to the health.

You basicly do the same as in step 6. Find out what accesses the value, look at the instruction and what probably is the base pointer value, and what is the offset, and already fill that in or write it down. But in this case the address you'll find will also be a pointer. You just have to find out the pointer to that pointer exactly the same way as you did with the value. Find out what accesses that address you found, look at the assembler instruction, note the probable instruction and offset, and use that.
and continue till you can't get any further (usually when the base address is a static address, shown up as green)

Click Change Value to let the tutorial access the health.
If you think you've found the pointer path click Change Register. The pointers and value will then change and you'll have 3 seconds to freeze the address to 5000

Extra: This problem can also be solved using a auto assembler script, or using the pointer scanner
Extra2: In some situations it is recommended to change ce's codefinder settings to Access violations when 
Encountering instructions like mov eax,[eax] since debugregisters show it AFTER it was changed, making it hard to find out the the value of the pointer





Extra3: If you're still reading. You might notice that when looking at the assembler instructions that the pointer is being read and filled out in the same codeblock (same routine, if you know assembler, look up till the start of the routine). This doesn't always happen, but can be really useful in finding a pointer when debugging is troublesome""",
    "step9": """Step 9: Shared code: (PW=31337157)
This step will explain how to deal with code that is used for other object of the same type

Often when you've found health of a unit or your own player, you will find that if you remove the code, it affects enemies as well.
In these cases you must find out how to distinguish between your and the enemies objects.
Sometimes this is as easy as checking the first 4 bytes (Function pointer table) which often point to a unique location for the player, and sometimes it's a team number, or a pointer to a pointer to a pointer to a pointer to a pointer to a playername. It all depends on the complexity of the game, and your luck

The easiest method is finding what addresses the code you found writes to and then use the dissect data feature to compare against two structures. (Your unit(s)/player and the enemies) And then see if you can find out a way to distinguish between them.
When you have found out how to distinguish between you and the computer you can inject an assembler script that checks for the condition and then either do not execute the code or do something else. (One hit kills for example)
Alternatively, you can also use this to build a so called "Array of byte" string which you can use to search which will result in a list of all your or the enemies players
In this tutorial I have implemented the most amazing game you will ever play.
It has 4 players. 2 Players belong to your team, and 2 Players belong to the computer. 
Your task is to find the code that writes the health and make it so you win the game WITHOUT freezing your health
To continue, press "Restart game and autoplay" to test that your code is correct


Tip: Health is a float
Tip2: There are multiple solutions""",
}


class healther:
    health = None
    last_decrement = None
    default_health = None
    random_to = None

    def __init__(self, health, random_to) -> None:
        super().__init__()
        self.default_health = health
        self.random_to = random_to
        self.reset()

    def reset(self) -> None:
        del self.health  # Нам нужно, что бы поменялся адрес, поэтому удаляем. Хз как сработает, TODO
        self.health = self.default_health
        self.last_decrement = 0

    def hit(self) -> bool:
        """ Return true if alive """
        self.last_decrement = random.randint(1, self.random_to)
        self.health -= self.last_decrement
        if self.health < 0:
            self.last_decrement -= self.health
            self.health = 0
        return self.health != 0


class Example(QWidget):
    step: int = 1
    refresh_rate: int = 40  # 40 Hz = 1000/40 ms = 25 ms

    def __init__(self):
        super().__init__()
        self.resize(450, 550)
        self.setWindowTitle('RE Tutorial')
        self.center()
        self.setLayout(QGridLayout())
        self.step1()
        self.show()

    def step1(self):
        self.layout_reset(self.layout())
        self.step = 1
        te_info = QTextEdit()
        te_info.insertPlainText(text["step1"])
        btn_continue = QPushButton(_('Continue'))
        self.layout().addWidget(te_info,      0, 0, 1, 3)
        self.layout().addWidget(btn_continue, 1, 0, 1, 3)

        btn_continue.clicked.connect(lambda: self.step2())

    def step2(self):
        self.layout_reset(self.layout())
        self.step = 2
        te_info = QTextEdit()
        te_info.insertPlainText(text["step2"])
        lbl_health_text = _('Health: {}')
        lbl_health = QLabel()
        btn_hitme = QPushButton(_('Hit me'))
        btn_next = QPushButton(_('Next'))
        btn_next.setEnabled(False)
        self.layout().addWidget(te_info,    0, 0, 1, 3)
        self.layout().addWidget(lbl_health, 1, 0, 1, 3)
        self.layout().addWidget(btn_hitme,  2, 0, 1, 3)
        self.layout().addWidget(btn_next,   3, 0, 1, 3)

        u = healther(100, 5)

        def on_btn_hitme():
            alive = u.hit()
            if not alive:
                reply = QMessageBox.question(self, _('Respawn?'), _("Your player is dead. Do you want to respawn?"), QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
                if reply == QMessageBox.Yes:
                    self.step2()

        def on_tick_handler():
            # print("tick, date:", datetime.datetime.now().strftime("%H:%M:%S"))
            if self.step != 2:
                return
            lbl_health.setText(lbl_health_text.format(u.health))
            if u.health > 100:
                btn_next.setEnabled(True)
            QTimer().singleShot(1000/self.refresh_rate, on_tick_handler)

        btn_hitme.clicked.connect(on_btn_hitme)
        btn_next.clicked.connect(lambda: self.step1())
        QTimer().singleShot(0, on_tick_handler)

    def step3(self):
        self.layout_reset(self.layout())
        self.step = 3
        te_info = QTextEdit()
        te_info.insertPlainText(text["step2"])
        lbl_health_text = _('Health: {}')
        lbl_health = QLabel()
        btn_hitme = QPushButton(_('Hit me'))
        btn_next = QPushButton(_('Next'))
        btn_next.setEnabled(False)
        self.layout().addWidget(te_info,    0, 0, 1, 3)
        self.layout().addWidget(lbl_health, 1, 0, 1, 3)
        self.layout().addWidget(btn_hitme,  2, 0, 1, 3)
        self.layout().addWidget(btn_next,   3, 0, 1, 3)

        u = healther(500, 10)

        def on_btn_hitme():
            alive = u.hit()
            if not alive:
                reply = QMessageBox.question(self, _('Respawn?'), _("Your player is dead. Do you want to respawn?"), QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
                if reply == QMessageBox.Yes:
                    u.reset()

        def on_tick_handler():
            # print("tick, date:", datetime.datetime.now().strftime("%H:%M:%S"))
            if self.step != 2:
                return
            lbl_health.setText(lbl_health_text.format(u.health))
            if u.health > 100:
                btn_next.setEnabled(True)
            QTimer().singleShot(1000/self.refresh_rate, on_tick_handler)

        btn_hitme.clicked.connect(on_btn_hitme)
        btn_next.clicked.connect(lambda: self.step1())
        QTimer().singleShot(0, on_tick_handler)

    def center(self):
        qr = self.frameGeometry()
        cp = QDesktopWidget().availableGeometry().center()
        qr.moveCenter(cp)
        self.move(qr.topLeft())

    def layout_reset(self, layout):
        if layout is not None:
            for i in reversed(range(layout.count())):
                layout.itemAt(i).widget().setParent(None)


def main():
    app = QApplication(sys.argv)
    ex = Example()
    return app.exec_()


if __name__ == '__main__':
    sys.exit(main())
