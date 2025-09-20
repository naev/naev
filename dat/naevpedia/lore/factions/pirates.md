---
title: "Galactic Space Pirates"
---
<%
    fwildones = faction.get("Wild Ones")
    fravenclan = faction.get("Raven Clan")
    fdreamerclan = faction.get("Dreamer Clan")
    fblacklotus = faction.get("Black Lotus")

    local strunknown = _("???")
    --strblacklotus = (fblacklotus:known()  and _("[Black Lotus](lore/factions/pirates/blacklotus)"))   or strunknown
    --strwildones   = (fwildones:known()    and _("[Wild Ones](lore/factions/pirates/wildones)"))       or strunknown
    --strravenclan  = (fravenclan:known()   and _("[Raven Clan](lore/factions/pirates/ravenclan)"))     or strunknown
    --strdreamerclan= (fdreamerclan:known() and _("[Dreamer clan](lore/factions/pirates/dreamerclan)")) or strunknown
    strblacklotus = (fblacklotus:known()  and _("Black Lotus"))  or strunknown
    strwildones   = (fwildones:known()    and _("Wild Ones"))    or strunknown
    strravenclan  = (fravenclan:known()   and _("Raven Clan"))   or strunknown
    strdreamerclan= (fdreamerclan:known() and _("Dreamer Clan")) or strunknown
%>
<%
wgtfct = require("naevpedia.lore.faction").init( "Pirate" )
%>
<widget wgtfct />

# Galactic Space Pirates

The Galactic Space Pirates are split into two main groups, the clans and independent pirates.
Among the independent pirates are marauders who are extremely aggressive and not very well-equipped.
Other pirates tend to be better equipped and usually strive to join one of the major pirate clans.

The four main clans are the <%= strblacklotus %>, <%= strwildones %>, <%= strravenclan %>, and <%= strdreamerclan %>.

## Organization

* **Leader:** Pirate Lords from the diverse clans
* **Leading Structure:** Pirate Assembly
* **Government:** Aristocracy (varies by clan)
* **Formation:** Beginning of History
* **Homeworld:** Varies by Clan

## Independent Pirates

Formed by individuals that have given up on society and turned to piracy to make a living.
They form a large part of all pirates and end up doing lots of the grunt work for the clans.
As long as they do not cause problems they are welcome at all pirate clansworlds.

## Marauders

Usually formed by individuals that are not fit by the clans due to extreme violence, no critical thinking, problematic behaviour, no hygiene, or all of the above.
They are tolerated by other pirates, who tend to exploit them and give them scraps, but they usually end up with short life spans.

## Pirate Assemblies

Usually occurring once a cycle, pirate assemblies are as formal of a gathering as you can find in the pirate world.
They are usually held at a more neutral pirate clansworld, such as those of the <%= strravenclan %>, and consist of several [decaperiods](mechanics/time) of partying and lawmaking.
The main event consists of the meeting of the Pirate Lords who will listen to proposals and decide on courses of actions, while letting their crew loose.

The assemblies tend to be a good opportunity for the Pirate Lords to calculate each other strengths, where they tend to bring significant ships from their fleet.
In many ways, the pirate assemblies tend to not only determine the future of the space pirates, but also tend to shape the future of the galaxy.

The events tend to also attract the attention of other actors in the galaxy, with most Great Houses and [the Empire](lore/factions/empire) sending somewhat undercover agents to try to get a glimpse of what is going on and gain advantage over other Great Houses.
It is also common for less scrupulous merchants to show up as it can be a very profitable opportunity to sell grog and equipment to the drunken pirates.

<% if fwildones:known() then %>
## Wild Ones Clan

Formed mainly by [Empire](lore/factions/empire) and [Soromid](lore/factions/soromid) outcasts, the Wild Ones are a violent and aggressive clan where strength is of utmost importance.
Weaker individuals tend to follow the stronger ones, and the strongest compete against each other.
That said, there are few deadly confrontations between clan members, as usually the weaker one will back out and submit when they see they don't have a chance.
They tend to not hold grudges, which allows for their society to work.

The clan's territory is the sparsely inhabited area between the [Empire](lore/factions/empire) and the [Soromid](lore/factions/soromid).
Their main clansworld was Haven, until it was destroyed in an offensive by the Empire and Great Houses.
Since then, they have moved to a more secluded area known as New Haven where they are careful to not repeat the same fate.

Although they get along well with the <%= strravenclan %>, they tend to get into fights with the <%= strblacklotus %>, and look down upon the <%= strdreamerclan %>.
<% end %>

<% if fravenclan:known() then %>
## Raven Clan

Arguably the clan that connects all the pirates and maintains the infrastructure for their success.
The Raven Clan is formed by many ex-merchants who were fed up with the corruption and bureaucracy of the Imperial system, and decided to do their own thing.
Although they deal in smuggling and black-market trade, they do not shun nor turn away from the occasional raid or assassination.
They are very diplomatic and put strong emphasis on human relationships, which they foster to maximize the success of their trading endeavours.

The Raven Clan mainly inhabits the Qorel tunnel, which is a chain of systems not accessible through standard jump points.
They have several bases along it and use their cunning and secrecy to supply goods and connect all the pirates together.
Due to their fundamental role, they tend to have strong connections and relationships with all the other pirate clans.
They also work with corrupt officials of the Great Houses, and pretty much anyone that has the credits to pay.
<% end %>

<% if fblacklotus:known() then %>
## Black Lotus

One of the most well organized pirate clans, the Black Lotus prides itself in methodology and organization.
Members follow strict recruitment policies to slowly go up in the ranks and gain more power in the hierarchy, very similar to large corporations.
Discipline is swift and punishment is carried out in public form to make sure everyone follows the rules, making them one of the most rigid pirate clans.

They inhabit the pocket of space between [House Za'lek](lore/factions/zalek) and [House Dvaered](lore/factions/dvaered), where they make a fortune from specializing in stolen research equipment and oddities from the universe.
They also tend to run protection rackets, which are often favoured over the whimsical [Dvaered](lore/factions/dvaered) Warlords and can provide more stability to the area.
While the price of these protection contracts is quite high, wealthy citizens have found that they are well worth the money, as any failed protection contract is met with swift discipline for those who failed, making failure rates extremely low.
<% end %>

<% if fdreamerclan:known() then %>
## Dreamer Clan

The newest of the main pirate clans, the Dreamer Clan was able to capitalize on [the Incident](lore/history/incident) and establish itself as a force to be reckoned with.
Formed mainly by refugees and outcasts of the so-called "civilized society", the Dreamer Clan is formed by individuals who do not want to follow rules.
They have no hierarchy and form an eclectic and anarchistic bunch, giving lots of leeway for personal freedom and using voting and other systems to decide how to take action.
Although they rely heavily on piracy, most individuals pursue artistic talents when not raiding nearby convoys or scavenging from wrecks from [the Incident](lore/history/incident).
They are also renowned for being a large hub of illicit substances, which their members use freely and some claim to get Sirius-like psychic powers from substance abuse.

They are located in the Nebula, near [House Sirius](lore/factions/sirius) and [the Frontier](lore/factions/frontier), making use of abandoned planets and stations, which they adapt to their purposes.
Although they tend to have little contact with other pirates outside of assemblies, they have lots of trade with the <%= strravenclan %>, which wants access to the lucrative drug trade.
Although in general less organized than other clans, they can be ruthless at raiding convoys, sometimes going deep into [House Sirius](lore/factions/sirius) space.
<% end %>
