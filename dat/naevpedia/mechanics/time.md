---
title: "Universal Synchronized Time (UST)"
---
# Universal Synchronized Time (UST)

Universal Synchronized Time (UST) is the standard time system used by human civilization.

## Explanation

UST consists of three basic components describing different amounts of time: the standard time unit (stu), which is equivalent to an Earth second; the period, which is equal to 10,000 stu (or earth seconds); and the cycle, which is equal to 5,000 periods (50,000,000 stu).
In colloquial usage, the terms "decaperiod" (equivalent to 10 periods) and "hectostu" (equivalent to 100 seconds) are also common.
UST dates are written in the form:

* UST C:PPPP.SSSS

Where "C" is the cycle, "PPPP" is the period (always displayed as four digits), and "SSSS" is the stu (always displayed as four digits).
So for example, the following hypothetical date indicates cycle 493, period 42, stu 2089 (which is about 100 cycles prior to [the Incident](lore/history/incident)):

* UST 493:0042.2089

When describing lengths of time, it is commonplace for computer systems to indicate a number of periods with either "p" or "stp", and a number of seconds with "stu".
This convention is not used for cycles, which are always spelled out in full as "cycles", but is also partially adopted in spoken form where periods are just referred as "periods", however, seconds are referred to as "stu" (pronounced like stew).
This also happens to solve the confusion between real-time seconds, and in-game seconds or stu.
Additionally, due to the metric nature of the time system, periods and seconds can be written out as a single unit, although in spoken conversation people report the periods and cycles separately.
Some examples:

* 783 stp (read as "783 periods")
* 42 stu (read as "42 stu")
* 12.0456 p (read as "12 periods and 456 stu")

The following is a chart of all time units used in Naev along with the corresponding Earth time unit they are similar to in terms of where they are used.

<%
    luatk = require "luatk"
    function tableust ( mw, tw )
        local data = {
            { "#n".._("UST unit"),"#n".._("Abbreviation"), "#n".._("Length of Time"), "#n".._("Equal to (in Earth time)"), "#n".._("Used like") },
            { _("Standard Time Unit"),     _([["stu"]]),        _("1 Earth second"), _("1 second"),                 _("Seconds") },
            { _("Hectostu"), _([[N/A]]),        _("100 seconds"),    _("1 minute and 40 seconds"),  _("Minutes") },
            { _("Periods"),     _([["p" or "stp"]]),        _("10,000 seconds"), _("2 hours and 47 minutes"),   _("Hours") },
            { _("Decaperiods"), _([[N/A]]),        _("10 periods"),     _("28 hours"),                 _("Days") },
            { _("Cycles"),      _([[N/A]]),        _("5,000 periods"),  _("579 days"),                 _("Years") },
        }
        return luatk.newTable( nil, 10, 0, tw-20, nil, data )
    end
%>
<widget tableust />

## Passaging of Time

Following is a list of actions and how much time they take.

* [Flying in space](mechanics/movement): For ships with a time dilation rate of 100% (that is, most small ships), time passes at a rate of 30 seconds per real-world second, which is why the GUI's clock increases by 0.01p every 3⅓ seconds. For ships with higher time dilation the passage of time is faster, and for ships with lower time dilation the passage of time is slower.
* [Landed](mechanics/landing): Time does not pass while landed.
* [Takeoff](mechanics/landing): Taking off takes 1 period, which means that stopping to refuel during time-sensitive missions is generally a bad idea.
* [Jumping](mechanics/hyperspace): Hyperspace jumps also take time, generally 1 period per jump, though some ships such as the [Quicksilver](ships/quicksilver) take less.
