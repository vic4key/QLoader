
# What is QLoader ?

**QLoader** is a PE loader creator that helps you quickly create a non-exe loader for application

## Features

* Easy to Create a non-exe Loader to avoid Anti-Virus Detection
* Easy to Define Patching Patterns under the JSON File Format
* Easy to Modify Patching Information (Target, Pattern, etc)
* Support both PE 32-bit & 64-bit
* Modify direct the created Loader for updating (do not need to `Save Project` for maintenance)
* Create Loader as
  - [X] Windows Shortcut .LNK
  - [X] Internet Shortcut .URL
  - [X] HTML Hyperlink `qloader: ...` (like `tel: ...`, `skype: ...`, etc)
* Patching Methods
  - [x] Fully Loaded
  - [x] At Entry Point
  - [ ] Unpacking (`TODO Later`)

## License

Released under the [MIT](LICENSE.md) license

## Tutorial

>*Note: Remember to turn on sub-title (CC) when watching*
>
>[![IMAGE_ALT](https://github.com/vic4key/QLoader/blob/master/QLoader/screenshots/video.png)](https://www.youtube.com/watch?v=nzyHh1CzNLM)

## User Interface

>![](QLoader/screenshots/ui.png?)

<details>
<summary>Pattern File (Template)</summary>

```
{
  "name": "this is a name",
  "brief": "this is a brief",
  "modules": [
    {
      "name": "user32.dll",
      "enabled": true,
      "patches": [
        {
          "name": "patch 1",
          "pattern": "11 22 ?? 44",
          "replacement": "12 21",
          "offset": "1h",
          "enabled": false
        },
        {
          "name": "patch 2",
          "pattern": "33 44 ?? ?? 77",
          "replacement": "34 43",
          "offset": "0x2",
          "enabled": true
        }
      ]
    },
    {
      "name": "kernel32.dll",
      "enabled": false,
      "patches": [
        {
          "name": "patch 3",
          "pattern": "55 66",
          "replacement": "56 65",
          "offset": "3",
          "enabled": true
        },
        {
          "name": "patch 4",
          "pattern": "77 88",
          "replacement": "78 87",
          "offset": "4h",
          "enabled": false
        },
        {
          "name": "patch 5",
          "pattern": "99 ?? AA BB CC DD FF",
          "replacement": "90 09",
          "offset": "0x5",
          "enabled": true
        }
      ]
    }
  ]
}
```
</details>

## Examples

<details>
<summary>Screenshots</summary>

> [Sublime Text 4 (4107 - 64-bit)](https://github.com/vic4key/QLoader/blob/master/QLoader/test/sublime%20text%204.json)
> 
> ![](QLoader/screenshots/sublime_text_4.png?)

> [Cute Video Converter (32-bit application)](https://github.com/vic4key/QLoader/blob/master/QLoader/test/cute%20video%20converter.json)
> 
> ![](QLoader/screenshots/cute_video_converter.png?)
</details>

> *Take a look at the [test](QLoader/test)* folder to see more examples

## Development

<details>
<summary>Required</summary>

> Installed `Visual Studio C++ 2019` or later
> 
> Installed [Vutils](https://github.com/vic4key/Vutils.git) library
</details>

<details>
<summary>Steps</summary>

> Step 1. Check the repo by command `git clone --recursive https://github.com/vic4key/QLoader.git`
> 
> Step 2. Open `QLoader.sln` in `MS Visual Studio` and start to work
</details>

## ToDo

* Support `Unpacking` patching method that using for complex packed/protected target
* Support builtin editor or a quick way for adding new patch entry

## Contact
Feel free to contact via [Twitter](https://twitter.com/vic4key) / [Gmail](mailto:vic4key@gmail.com) / [Blog](https://blog.vic.onl/) / [Website](https://vic.onl/)
