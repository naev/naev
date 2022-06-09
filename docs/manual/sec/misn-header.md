## Headers
\label{sec:misn-headers}

Headers contain all the necessary data about a mission or event to determine where and when they should be run. They are written as XML code embedded in a Lua comment at the top of each individual mission or event. In the case a lua file does not contain a header, it is ignored and not loaded as a mission or event.

\vspace{3mm}

| Location | Event | Mission | Description |
| --- |:---:|:---:| --- |
| none | ✔ | ✔ | Not available anywhere. |
| computer | | ✔ | Available at mission computers.\\ `accept` function is run when run accepted from the mission computer interface. |
