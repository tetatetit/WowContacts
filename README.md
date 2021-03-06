
# Test project/task for [WowApp](https://www.wowapp.com/)

This is test project/task I have implemented for [WowApp](https://www.wowapp.com/) company by [`./c++ wowapp-task.pdf`](./c++%20wowapp-task.pdf) tech spec and expectations they provided while applied for remote contractor role for them. My initial estimate for this task was 10 hours but while started to implement - have seen that I have underestimated so it have taken ~20 hours for me. 

I have contacted just by Ukrainian recruiter who said that acts on behalf of that company and searching Senior Qt Developer for WowApp project and proposed very attractive conditions but first stage was to implement test task. I decided to risk and started that task since it really appeared very interested for me just to refresh my knowledges/skills with Qt since I worked lot with it earlier and would be interesting in some new contracts on Qt projects.

I have not signed any NDA with recruiter who contacted me (so even started to doubt he really acted on behalf of that company) as well as not gave any other agreement on non-disclosure in neither verbal nor written nor any other form. So I don't see any legal/moral/ethic reason/restriction to do not publish this task and everything about this srory. **While I would highly appreciate any feedback in issues section of this repo (any critism to my solution or any confirmation that reviewers of my task was correct at least in something)**

## My clarififcation questions and their answers (before start)

To provide estimates and start task I would like to clarify following:

1. "lazy loading":

    * 1.1. Are there requirement to keep in memory only those contacts which are required to display/scroll at particular moment while keeping all contacts stored on the disk already parsed from json and sorted in desirable order (e.g. in the flat binary cache file, QContiguousCache or so)? So that only necessary portion (page/range) of contacts would be loaded and then freed up from the memory once not needed anymore.

        > contacts already loaded will not be "unloaded" for the entire lifetime of the app
    
    * 1.2. Or is it assumed/acceptable to keep all contacts completely in memory (again already parsed and sorted) while lazy-loading to GUI only those required to display/scroll at particular moment (i.e. just to keep GUI resources just enough minimal for only actually displayed contacts, while keeping contacts data themself whole in memory for speed up search etc., see my next question)
    
        > so I'd say that option 1.2 is acceptable,
    
2. "filter data based on a user’s
input.": 

    * 2.1. In case of 1.1 above - we definitely need extra search (word) indexing cache. Otherwise filtering will be quite ineffective by speed and disk usage.
    
    * 2.2. In case of 1.2 - performance might be reasonably acceptable without word indexing cache but still cache (in file or in memory) would allow to reach highest possible performance (less noticeable by speed on modern CPU but rather CPU usage itself).
    
        > and so 2.2
      
3. Another golden mean by all aspects of performance (CPU, disk, RAM and overall user expirience by speed etc) for both 1 and 2 would be to use **sqlite** with Qt plugin which would allow to keep in memory the only portion of contacts and even the only portion of their data required to scroll/display in particular moment without necessity of any cache, even for search indexing as well. While this would simplificate the task a lot in this way.

    > #### but if you want to implement 3 it's absolutely fine too.
    > You choose what you want to implement, that's why you have to estimate.
      
4. Behaviour expectation of "filter data based on a user’s input." need to be clarified as well.

    * 4.1. Is it expected to look for typed word in all fields of contact or only in particular? E.g. it can be looked only in firstName and lastName only or in country and group as well.

        > Search is enough for FirstName, LastName and username so that if we have a user "John Smith" having username "jsmith" we can search by either "John", "Smith" or "jsmith" and it should find this contact in all cases.
      
    * 4.2. Is it expected to look only from the beginning of word or from the any part of it?  E.g. when suppose first name is Serhiy and last name Berezin, then typing  only "erh" or "ere" would display that contact. I checked how standard (build in) Android Contacts app behaves in this and found it's does not display mentioned contact in this case but only when to start type first or last name from the beginning. Personally me as Android Contacts app user - I truly lack that my input is not looked in the **middle of contact fields**. But as for developer, in case if 3 above (sqlite) is not acceptable. then it requires either additional work (on caching in case of 1.1) or would degrade some performance (in case of 1.2).

         > #### we don't need to search in the **middle of the word**.
         > Beginning is enough. 'mith' used as search term should not return "Smith", but 'Smi' or 'Jo' or 'js' should.
    
    * 4.3. Is it expected to handle multiple worlds in typed filter string or just one? In case of multiple it could look for each word matching each field as defined in 4.1 for matching as defined in 4.2. This how android Contacts app behaves. I understand that without multiple - filtering would be incomplete for real world usage but just checking requirement whether requirement for test task are lower. Since it also might decrease time of test task completion.

        > yes, multiple search terms should be used with the semantic of operation AND, meaning, if we have 2 users "John Smith" and "James Smith" and we input "John Smith" as search term then only "John Smith" should be returned not both. If we use "Smith J" both should be displayed, if we continue and add 'o' and search term becomes "Smith Jo" then only "John Smith" must be displayed.
    
    * 4.4. And for 4 here again solution 3 (sqlite) would be golden mean since would handle most of above job on itself. I.e. for for querying by any fields from either beginning or the middle etc)
        > As said before,
        > #### it's perfectly fine to implement with **sqlite**
        > if you wish, just take into account when you estimate.
    
## Their feedback on implemented task

Very over-enigeered solution, that actually implemented the Least from our test task. Code is too hard to follow, used lots of un-needed threads, **sqlite**, while not having finished with direct tasks. Overcomplicated multithreading in Qt and C++ is very dangerous, since it may lead to very subtle bugs. Qt Concurrent module that is most optimal for these kinds of tasks was not used. Also “Search/Filter” is broken, e.g. users cannot search **“mid-word”** that is provided by Qt out of the box. No UI to display contact Avatar and additional info.

### And another in Ukrainian same day later

##### For Ukrainian speaking or who just courious to translate with e.g. Google Translate

ну от скачав я його код в нову чисту, і він не компілиться по-людськи. Я подумав-подумав, пошаманив із настройками середовища. Пішло. Далі пошук по контактах у нього некоректно працює , 3 контакти у "favorites" бракує. Групи "Saved groups" взагалі нема. нема пошуку з "середини слова" (наприклад Alex Bannyk не знайдеться якщо шукати "lex").

## My comments to their feedback

> Very over-enigeered solution

Examples in the code?

> while not having finished with direct tasks

What not finished? Looks like viewed some not up to date code

> Code is too hard to follow

Examples in the code or overall?

> used lots of un-needed threads,

Where "lots"  while only 2 other then main GUI:
 1. Storage which fulfills all storage queries (search/filter/sort, update)
 2. Fetcher which downloads and parses contacts roster in JSON

 Where "un-needed"  while both above operations could lag in case of running in GUI thread and huge amount of contacts and/or slow/loaded CPU/IO (disk/network)?

 > **sqlite**
 
No comments, just quoting and referencing their answers above on my clarification questions
* > [3 (but if you want to implement 3 its absolutely fine too)](#but-if-you-want-to-implement-3-its-absolutely-fine-too)
* > [4.4 (its perfectly fine to implement with sqlite)](#its-perfectly-fine-to-implement-with-sqlite)  
 
 
> Overcomplicated multithreading in Qt and C++ is very dangerous

Where is "overcomplicated" while 
1. both threads has absolutely clear and streighforward life-cycle: one (storage) permenent with it's own loop and another (fetcher) runs only to download of contacts roster and then self-deletes with deleteLater(). 
2. Both threads communicates with main (GUI) thread/objects exclusively by queued signal-slot mechanism _without_ sharing any data/objects/pointers what eliminates necessity of any synchronization, i.e. fulfilling all requests by threads are performed asynchroneously in strict queue (FIFO) manner.
3. All above implemented with only standard Qt "out of the box" features (signals, slots, thread loop, `moveToThread()`, `deleteLater()`)

> Qt Concurrent module that is most optimal for these kinds of tasks was not used

Any authoriative background/reference why it is "most optimal" for any similar pattern to "these kinds of tasks"? Since doesn't even name "Qt Concurrent" is self-explanatory that it's primarily rather for **concurent**/**parallel**, along with https://doc.qt.io/qt-5/qtconcurrent-index.html stating "automatically adjust the number of threads used according to the number of processor cores available" and "to scale when deployed on multi-core systems"? While our patterns for both "storage" and "fetcher" are just worker threads performing all operation in the FIFO queue and tighly communicating with GUI thread asynchroneously (via non-blocking queued signals-slots), what is much more common pattern for most IO operations. And while "concurrent/parallel" computing is most common for heavy computations running in the background in parallel using most from multi-core architecture. That is why QThread emits signals for this (start, finished) and implementing thread loop for handling received signals in quaue. And good example of correct choice and usage by Qt team itself - that QNetworkAccessManager runs single `QThread` based working thread internally and communicating with caller thread by signal-slots mechanism asynchronously. Why they used `QThread` but not `QtConcurrent` if it would be more appropriate for this? Also `QThread` has reliable "out of the box" mechanism to be interrupted gracefully with `quit()` slot or `exit()` member together with `isInterruptionRequested()` checked periodically from long running operation.

`QtConcurrent` also have capability to communicate with GUI via `QFutureWatcher` and I considered this approach as well but found it requirying more things to take care for in our case than just to use `QThread` + `QObject` derived clases with all requried functionality "out of the box" (mentionted above)

> Also “Search/Filter” is broken, e.g. users cannot search “mid-word” that is provided by Qt out of the box

> users cannot search “mid-word”

No comments, just quoting and referencing their answer above on my clarification questions

> [4.2 (we dont need to search in the middle of the word)](#we-dont-need-to-search-in-the-middle-of-the-word)

> Also “Search/Filter” ... that is provided by Qt out of the box

If you about  QSortFilterProxyModel - in this case filtering would work only with part (range) of items already lazy-loaded to model, but not all items in the roster. Or what else do you mean?

> No UI to display contact Avatar and additional info.

Reviewed not up to date code since first time provided them with code where above was really unimplemented yet but with clear note about this as well as about that they can review what was implemented on that time for while and about that fully complete code will be sent them within next few days what was fulfilled by me timely (i.e. the code with working contact Avatar and additional info) what is viable by commits history of this repo.

### And that last part in Ukrainian

For Ukrainian speaking or who just courious to translate with e.g. Google Translate

Alex Bannyk (Simply Recruiter)  6:31 PM
Фидбек от разработка: ну от скачав я його код в нову чисту, і він не компілиться по-людськи. Я подумав-подумав, пошаманив із настройками середовища. Пішло. Далі пошук по контактах у нього некоректно працює , 3 контакти у "favorites" бракує. Групи "Saved groups" взагалі нема. нема пошуку з "середини слова" (наприклад Alex Bannyk не знайдеться якщо шукати "lex").

Alex Bannyk (Simply Recruiter)  6:32 PM
На этом хотелось бы поставить точку и еще раз поблагодарить тебя.

Serhiy Berezin  7:56 PM
> 3 контакти у "favorites" бракує. Групи "Saved groups" взагалі нема

Фікснув за 15 хв і закомітив. Трівіальний баг. Наче по ТЗ були зовсім інші очікування і критерії оцінки ніж по відсутності самих банальних трівіальних багів , які вилазять на самих простих тестах ручних чи автоматичних на саммих ранніх етапах і фіксяться за хвилини? Чим може реальний приклад наведете де така "неуважність" шкодить реальному життєвому циклу реального продукта при розробці (чиюсь продуктивність знижує чи якість продукта)? Чи може ревювер сам ніколи не робив таких трівіальних багів (особливо коли на задачу обмежений час виділяється), якщо дійсно працює в команді реального продукта а не виключно ревювить для рекрутерів

Serhiy Berezin  8:11 PM
> і він не компілиться по-людськи

Це хлопці, серйозно теж мінус при оцінці вважаєте? Таке теж виявляється за кілька хвилин коли на кількох платформах компілиться і теститься одночасно. Просто я лиш на своїй робочі Дебіан 10 поставив той Кюті 5.11 що в дистрибютиві і створив  стандартний проект по замовчуванню візардом. Дивно що він не компілиться в ревювера відразу. Хоч я в себе начисто клонував з гіта в іншу папку і все компілиться. Може він не зконфігурував його спочатку (В QtCreator в Projects куди воно саме закидує перше при відкритті проекта перший раз начисто коли .user файл відсутній, що лиш кнопку Configure нажати і там Кіт вибрати...чи просто qmake викликати із кореня проекта - я думав це будь який кютішнік знає)

Alex Bannyk (Simply Recruiter)  8:14 PM
Очевидно, что ваши подходы принципиально отличаются и это будет мешать совместной работе.

Serhiy Berezin  8:15 PM
> наприклад Alex Bannyk не знайдеться якщо шукати "lex"

Оце вже по котрому колу пішло? Ще раз процитувати те що мені на мої уточнюючі запитання перед початком роботи відповіли? Що ВОНО І НЕ МАЄ ШУКАТИ з середини а ЛИШЕ З ПОЧАТКУ. Так до речі Контакти гугловські працюють на мобільних пристроях і в вебі - можете протестити. Але схоже гуглу і його розробникам далеко до вимог ревюверів ....


## My cover notes (after implemented)

Here my cover notes I provided to them together with implemented task

To download and display contacts: “Press to download single button”

* “TODO:” comments added where I provided flaws/drawbacks and how it would be potentially improved if more time allowed so can be found by global search by cpp/h
files.
* Filtering is also done fully in separate thread communicating with GUI thread only by
signal⇔ slot mechanism asynchronously and queued manner, i.e. never blocking GUI
thread regardless of how many contacts are. The same for downloading, updating and
querying contact details. All processes (e.g. Downloading, Filtering etc. ) are reflected in
status-bar.
* Everywhere provided just enough minimal implementation to meet tech-spec while save
time and while tried to design everywhere in the way it could be easily extended
improved near infinitely :) without stuck on something and/or without redesign/rewrite
any essential part of what already done.
    * Any static forms/views customization (like contact form) can be done via form Designer or manually (actually same but more advanced what form designer does not support, or even using QML if to move/decouple UI a little from controller.
    * All another custom model remapping/transformation for different kind of view/representation can be done via model proxies abstraction layer what is demonstrate
    * All another kind of contacts queries/retrieval can be easily added to storage abstraction layer or even storage can be internally redo from SQLite to anything else (e.g. some flat file or cache).
    * All another custom drawing can be changed/added via delegates.
    * Also another view like QListView can be used or more advanced customized based on QAbstractItemView
    * Lazy loading is implemented out of the box as part of QSqlQueryModel and works perfectly for this task in the term of memory and performance (just prefetches from SQL query result by 256 rows each time scroll reaches end and stores prefetched rows to some internal SQLite storage - either locally on disk inside sqlite database file or in the memory (did not investigate too much about for now). It could be easily reimplementing by subclassing QAbstractTableModel or QAbstractItemModel for more advanced customizations if time would allow and if would give real advantages in the context of this task - while using QSqlQueryModel and other Qt stuff as hint since available in source code like e.g. https://code.qt.io/cgit/qt/qtbase.git/tree/src/sql/models/qsqlquerymodel.cpp?h=dev (easily found by just googling e.g. “qsqlquerymodel.cpp”)
    * Some decoupling could be done still more thoroughly in some cost of time but not sure they worth on such stage since also later can be easily recactored just following same general direction/approaches/patterns
    * Progress indicator for downloading. UI beatifying, fonts, icons, graphics etc. as well as other bells and whistles can be added easily and quickly in any direction basing on already implemented app design/architecture.
    * Generating (drawing) avatar and providing them (via DecorationRole) to contact list (QTableView currently) as QImage - I decided to implement in the model(ContactModel) in favour of just drawing it in delegate because it is more logical/straightforward/obvious for rather model than for delegate to know whether to generate avatar basing on user photo (if implemente +available what is easier implement in the future with such approach) or just basing on first and last name (like it’s already implemented for now). Also whether to generate (draw) avatar on each data() call saving memory but taking more CPU or just to generate avatars for all prefetched (lazy loaded) rows keeping them in the memory. And finally - drawing of avatar can be easily moved to delegate if will be decided later as more optimal.
    * Contact list can be easily improved by proxying current ContactModel to represent (transform) first+last name columns into single column while avatar into decoration of that item. This would allow us to move forward with replacing QTableView with QListView which would show everything currently displayed out of the box including groups (i.e. preserving all existing functionality) while to allow infinitely flexible representation/styling/drawing/styling via subclassing of QStyledItemDelegate. Now it’s also possible with current ContactModel but less flexible due of table column layout, i.e. redraw is bound to rect of particular cell that paint() is called for and the data/decoration provided by model for it
    * Ideally would redesign ContactStorage class that it would provide results of filter not as QSqlQuery but as just a QList with limited number of items (e.g. 256) while then model would send signal to ContactStorage (directly connected or via controller) to fetch more items for the same filter/query which would provided by ContactStorage asynchronously as another signal which would add another 256 items to the model (again directly or via controller). In this way we would fully abstract storage and model from SQL allowing to replace it with any other kind of storage/cache internally without even touching model and storage public methods, while keep everything async non-blocking with near same grade of performance and memory saving (just slightly more due each 256 items are loaded to memory while with SQLite they might be retrieved from the disk depending it driver implementation in QT)
    * Spent some more time and delayed due trying to be too much idealistic in design, architecture, and minimalistic in term to chose simplest/easiest approach, reusing existing stuff provided by Qt (plus SQLite as got approval) just to do not reinvent the weels and bicycles (or sometime airplanes) especially when they already perfectly done and tested by others.
* Error handling still todo almost everywhere due of lack of time. While asserts are used whenever important to catch anything that would go wrong from outside worlds (may be also  still more asserts need to be added for catching internal mistakes/bugs if to look and walk through code).
* Lost some time on trying to remap ContactModel to be displayed in QListView where each contact group is represented as collapsable/expandable parent which but it does not work well with table representation of data in the term of lazy loading (fetchMore() called on each group  expanded/collapsed but not only when scrolling which cause most of rows becomes prefetched on just start)
* If there would more time - I would also improve overall UX/UI (i.e including graphics  and visual design) as well

Please feel free to request any improvements in any direction to see my approach, steps and direction
