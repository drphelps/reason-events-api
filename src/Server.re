[@bs.module "postgraphile"]
external postgraphile : (string, string, 'a) => Express.Middleware.t = "";

[@bs.module] external cors : 'config => Express.Middleware.t = "";

[@bs.module "./Plugins"] external stripNullsFromDefaultFields : 'a => 'b = "";

let debug = Debug.make("reason-events", "Server");

let debugExn = Debug.make("reason-events", "Server:exn");

/**
 * Report listen status
 */
let onListen = exn =>
  switch (exn) {
  | exception (Js.Exn.Error(err)) =>
    debug("Express listen error");
    debugExn(err);
  | _ =>
    Js.log(
      Chalk.blue("reason-events")
      ++ " is listening on port "
      ++ Chalk.green(Js.Int.toString(Config.Server.port)),
    )
  };

/**
 * The start routine for the application server
 */
let main = () =>
  MnstrServer.Http.make(
    ~middleware=
      default =>
        default
        @ [
          Use(cors({"exposedHeaders": Config.Server.corsHeaders})),
          /* Handle GraphQL requests */
          Use(
            postgraphile(
              Config.Database.url,
              "events",
              {"appendPlugins": [|stripNullsFromDefaultFields|]},
            ),
          ),
        ],
    ~onListen,
    ~isDev=Config.Server.isDev,
    ~port=Config.Server.port,
    (),
  );

/**
 * If this module was run from the command line, execute the main() routine
 */
if ([%bs.raw "require.main === module"]) {
  main() |> ignore;
};
