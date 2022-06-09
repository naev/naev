## Headers
\label{sec:misn-headers}

Headers contain all the necessary data about a mission or event to determine where and when they should be run. They are written as XML code embedded in a Lua comment at the top of each individual mission or event. In the case a lua file does not contain a header, it is ignored and not loaded as a mission or event.

The valid location parameters are as follows:

\vspace{3mm}

| Location | Event | Mission | Description |
| --- |:---:|:---:| --- |
| none | ✔ | ✔ | Not available anywhere. |
| land | ✔ | ✔ | Run when player lands |
| enter | ✔ | ✔ | Run when the player enters a system. |
| load | ✔ | | Run when the game is loaded. |
| computer | | ✔ | Available at mission computers. |
| bar | | ✔ | Available at spaceport bars. |

\vspace{3mm}

Note that availability differs between events and missions. Furthermore, there are two special cases for missions: `computer` and `bar` that both support an `accept` function. In the case of the mission computer, the `accept` function is run when the player tries to click on the accept button in the interface. On the other hand, the spaceport bar `accept` function is called when the NPC is approached. Note that this NPC must be defined with `misn.setNPC` to be approachable.
