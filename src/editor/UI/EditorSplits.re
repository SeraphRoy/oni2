/**
   Editor Splits

   This modules interprets the current state of the window
   manager and arranges its children accordingly
 */
open Revery;
open UI;
open Oni_Model;
open Oni_Core.Types.EditorSplits;

let component = React.component("EditorSplits");

/**
   TODO: width and height should be used as percentages of the
   window size not as direct values
 */
let getSplitStyle = ({layout, height: h, width: w, _}) =>
  Style.(
    switch (layout) {
    | Full => [top(0), bottom(0), flexGrow(1)]
    | VerticalLeft => [top(0), bottom(0), width(w)]
    | VerticalRight => [top(0), bottom(0), width(w)]
    | HorizontalTop => [top(0), bottom(0), height(h)]
    | HorizontalBottom => [top(0), bottom(0), height(h)]
    }
  );

let splitContainer = Style.[flexGrow(1), flexDirection(`Row)];

let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks =>
    (
      hooks,
      <View style=splitContainer>
        ...{
             WindowManager.toList(state.windows.splits)
             |> (
               splits =>
                 List.mapi(
                   (index, split) => [
                     <View style={getSplitStyle(split)}>
                       {split.component()}
                     </View>,
                     <WindowHandle
                       splits
                       layout={split.layout}
                       windowNumber=index
                       theme={state.theme}
                     />,
                   ],
                   splits,
                 )
                 |> List.flatten
             )
           }
      </View>,
    )
  );