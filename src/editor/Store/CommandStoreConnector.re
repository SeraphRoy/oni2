open Oni_Core;

open Oni_Model;
open Oni_Model.Actions;

let start = _ => {
  let singleActionEffect = (action, name) =>
    Isolinear.Effect.createWithDispatch(~name="command." ++ name, dispatch =>
      dispatch(action)
    );
  let multipleActionEffect = (actions, name) =>
    Isolinear.Effect.createWithDispatch(~name="command." ++ name, dispatch =>
      List.iter(v => dispatch(v), actions)
    );

  let closeEditorEffect = (state, _) =>
    Isolinear.Effect.createWithDispatch(~name="closeEditorEffect", dispatch => {
      let editor =
        Selectors.getActiveEditorGroup(state) |> Selectors.getActiveEditor;

      switch (editor) {
      | None => ()
      | Some(v) => dispatch(ViewCloseEditor(v.editorId))
      };
    });

  let splitEditorEffect = (state, direction, _) =>
    Isolinear.Effect.createWithDispatch(~name="splitEditorEffect", dispatch => {
      let buffer = Selectors.getActiveBuffer(state);

      let newEditorGroup =
        switch (buffer) {
        | Some(b) =>
          let ec = EditorGroup.create();
          let (g, editorId) =
            EditorGroup.getOrCreateEditorForBuffer(ec, Buffer.getId(b));
          let g = EditorGroup.setActiveEditor(g, editorId);
          g;
        | None => EditorGroup.create()
        };

      dispatch(EditorGroupAdd(newEditorGroup));

      let split =
        WindowManager.createSplit(
          ~direction,
          ~editorGroupId=newEditorGroup.editorGroupId,
          (),
        );

      dispatch(AddSplit(split));
    });

  let toggleExplorerEffect = ({fileExplorer, _}: State.t, _) => {
    Isolinear.Effect.createWithDispatch(~name="explorer.toggle", dispatch => {
      let action =
        fileExplorer.isOpen
          ? RemoveDockItem(WindowManager.ExplorerDock)
          : AddDockItem(WindowManager.ExplorerDock);
      dispatch(action);
    });
  };

  let commands = [
    (
      "commandPalette.open",
      _ =>
        multipleActionEffect([
          MenuOpen(CommandPalette.create),
          SetInputControlMode(TextInputFocus),
        ]),
    ),
    ("quickOpen.open", _ => singleActionEffect(QuickOpen)),
    (
      "menu.close",
      _ =>
        multipleActionEffect([
          MenuClose,
          SetInputControlMode(EditorTextFocus),
        ]),
    ),
    (
      "menu.open",
      _ =>
        multipleActionEffect([
          MenuClose,
          SetInputControlMode(EditorTextFocus),
        ]),
    ),
    (
      "menu.next",
      _ =>
        multipleActionEffect([SetInputControlMode(MenuFocus), MenuNextItem]),
    ),
    (
      "menu.previous",
      _ =>
        multipleActionEffect([
          SetInputControlMode(MenuFocus),
          MenuPreviousItem,
        ]),
    ),
    (
      "menu.select",
      _ =>
        multipleActionEffect([
          MenuSelect,
          SetInputControlMode(EditorTextFocus),
        ]),
    ),
    ("view.closeEditor", state => closeEditorEffect(state)),
    ("view.splitVertical", state => splitEditorEffect(state, Vertical)),
    ("view.splitHorizontal", state => splitEditorEffect(state, Horizontal)),
    ("explorer.toggle", state => toggleExplorerEffect(state)),
  ];

  let commandMap =
    List.fold_left(
      (prev, curr) => {
        let (command, handler) = curr;
        StringMap.add(command, handler, prev);
      },
      StringMap.empty,
      commands,
    );

  let updater = (state: State.t, action) => {
    switch (action) {
    | Command(cmd) =>
      switch (StringMap.find_opt(cmd, commandMap)) {
      | Some(v) => (state, v(state, cmd))
      | None => (state, Isolinear.Effect.none)
      }
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
