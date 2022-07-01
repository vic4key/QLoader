
# What is QLoader ?

**QLoader** a.k.a **Quick Loader** is a PE loader that helps you quickly create a loader for application

## Features
* Define patterns under the JSON File Format
* Create Loader as Windows Shortcut .LNK
* Create Loader as Internet Shortcut .URL
* Create Loader as HTML Hyperlink `qloader: ...` (like `tel: ...`, `skype: ...`, etc)

## License

Released under the [MIT](LICENSE.md) license

## Development

> Step 1. Required Visual Studio C++ 2019 or later.
> 
> Step 2. Required [Vutils](https://github.com/vic4key/Vutils.git) library
> 
> Step 3. Check [this](https://github.com/vic4key/QLoader.git) repository and start to work.

>User Interface
>
>![](QLoader/screenshots/ui.png?)

>Pattern File (Template)

```
{
  "name": "this is a name",
  "brief": "this is a brief",
  "modules":
  [
    {
      "name": "user32.dll",
      "enabled": true,
      "patches": [
        {
          "name": "patch 1",
          "pattern": "11 22 ?? 44",
          "replacement": "12 21",
          "offset": 1,
          "enabled": false
        }
      ]
    },
    {
      "name": "kernel32.dll",
      "enabled": false,
      "patches":
      [
        {
          "name": "patch 3",
          "pattern": "55 66",
          "replacement": "56 65",
          "offset": 3,
          "enabled": true
        },
        {
          "name": "patch 4",
          "pattern": "77 88",
          "replacement": "78 87",
          "offset": 4,
          "enabled": false
        }
      ]
    }
  ]
}
```

## Examples

> *Take a look at the [test](QLoader/test)* folder

>Sublime Text 4 (64-bit application)
>
>![](QLoader/screenshots/ui-sublime_text_4.png?)
>
>![](QLoader/screenshots/sublime_text_4.png?)

>Cute Video Converter (32-bit application)
>
>![](QLoader/screenshots/ui-cute_video_converter.png?)
>
>![](QLoader/screenshots/cute_video_converter.png?)

>Etc

## Contact
Feel free to contact via [Twitter](https://twitter.com/vic4key) / [Gmail](mailto:vic4key@gmail.com) / [Blog](https://blog.vic.onl/) / [Website](https://vic.onl/)