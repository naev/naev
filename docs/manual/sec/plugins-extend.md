## Extending Naev Functionality \naev

This section deals with some of the core functionality in Naev that can be extended to support plugins without the need to be overwritten. Extending Naev functionality, in general, relies heavily on the [Lua](https://www.lua.org/) using custom bindings that can interact with the Naev engine.

A full overview of the Naev Lua API can be found at [naev.org/api](https://naev.org/api) and is out of the scope of this document.

### Adding News \naev

News is controlled by the [dat/events/news.lua](https://github.com/naev/naev/blob/main/dat/events/news.lua) script. This script looks in the [dat/events/news/](https://github.com/naev/naev/tree/main/dat/events/news) for news data that can be used to create customized news feeds for the player. In general, all you have to do is created a specially formatted news file and chuck it in the directory for it to be used in-game.

When the player loads a game, the news script goes over all the news data files and loads them up. Afterwards, each tim ethe player lands, it creates a dynamic list of potential news based on the faction and characteristics of the landed spob. Afterwards, it randomly samples from the news a number of times based on certain criteria. News is not refreshed entirely each time the player lands, instead it is slowly updated over time based on a diversity of criteria. When new news is needed, the script samples from the dynamic list to create it. Thus it tends to slowly evolve as the player does things.

Let us take a look at how the news data files have to be formatted.

At the core, each news data file has to return a function that returns 4 values:

1. The name of the faction the news should be displayed at, or `"Generic"` for all factionts with the `generic` tag.
2. The headers to use for the faction. Set to nil if you don't want to add more header options.
3. The greetings to use for the faction. Set to nil if you don't want to add more greeting options.
4. A list of available articles for the faction.

Let us look at a minimal working example with all the features:

```lua
local head = {
   _("Welcome to Universal News Feed.")
}
local greeting = {
   _("Interesting events from around the universe."),
}
local articles = {
    {
      head = N_([[Naev Dev Manual Released!]]),
      body = _([[The Naev Development Manual was released after a long time in development. "About time" said an impatient user.]]),
    },
}
return function ()
   return "Independent", head, greeting, articles
end
```

The above example delares 3 tables corresponding to the news header (`head`), news greeting (`greeting`), and articles (`articles`). In this simple case, each table only has a single element, but it can have many more which will be chosen at random. The script returns a function at the bottom, that returns the faction name, `"Independent"` in this case, and the four tables. The function will be evaluated each time the player lands and news has to be generated, and allows you to condition what articles are generated based on all sorts of criteria.

Most of the meat of news lies in the articles. Each article is represented as a table with the following elements:

1. `head`: Title of the news. Must be an untranslated string (wrap with `N_()`)
2. `body`: Body text of the news. Can be either a function or a string. In case of being a function, it gets evaluated.
3. `tag` (optional): Used to determine if a piece of news is repeated. Defaults to the head, but you can define multiple news with the same tag to make them mutually exclusive.
4. `priority` (optional): Determines how high up the news is shown. Smaller values prioritize the news. Defaults to a value of 6.

As an alternative, it is also possible to bypass the news script entirely and directly add news with [`news.add`](https://naev.org/api/modules/news.html#add). This can be useful when adding news you want to appear directly as a result of in-game actions and not have it randomly appear. However, do note that not all players read the news and it can easily be missed.

### Adding Bar NPCs \naev

TODO

### Adding Derelict Events \naev

TODO add engine support

### Adding Points of Interest \naev

TODO

### Adding Personalities \naev

TODO
