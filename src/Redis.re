type existence =
  | XX
  | NX;

type expiration =
  | EX(int)
  | PX(int);

module Cursor = {
  type t =
    | FirstCursor
    | NextCursor(string)
    | LastCursor;

  let start = FirstCursor;

  let isLast = cursor => cursor == LastCursor;
};

module Internal = {
  type client;

  module JsError = {
    module Command = {
      [@bs.deriving abstract]
      type t =
        pri {
          [@bs.optional]
          args: array(Js.Json.t),
          [@bs.optional]
          name: string,
        };
    };

    [@bs.deriving abstract]
    type t =
      pri {
        [@bs.optional]
        name: string,
        [@bs.optional]
        message: string,
        [@bs.optional]
        stack: Js.Json.t,
        [@bs.optional]
        command: Command.t,
        [@bs.optional]
        code: string,
        [@bs.optional]
        buffer: string,
        [@bs.optional]
        offset: int,
        [@bs.optional]
        origin: Js.Exn.t,
      };

    let argsGet = error =>
      commandGet(error)->Belt.Option.flatMap(Command.argsGet);

    let commandGet = error =>
      commandGet(error)->Belt.Option.flatMap(Command.nameGet);
  };

  type promise = Repromise.Rejectable.t(Js.Json.t, JsError.t);

  let argsWithExistence = (value, args) => {
    switch (value) {
    | Some(NX) => Belt.Array.concat(args, [|"NX"|])
    | Some(XX) => Belt.Array.concat(args, [|"XX"|])
    | None => args
    };
  };

  let argsWithExpiration = (value, args) => {
    switch (value) {
    | Some(EX(seconds)) =>
      Belt.Array.concat(args, [|"EX", string_of_int(seconds)|])
    | Some(PX(seconds)) =>
      Belt.Array.concat(args, [|"PX ", string_of_int(seconds)|])
    | None => args
    };
  };

  let argsWithCursor = (value, args) =>
    switch (value) {
    | Cursor.FirstCursor => Belt.Array.concat(args, [|"0"|])
    | Cursor.NextCursor(cursor) => Belt.Array.concat(args, [|cursor|])
    // TODO: Verify that using the LastCursor again doesn't error in Redis
    | Cursor.LastCursor => Belt.Array.concat(args, [|"0"|])
    };

  let argsWithMatch = (value, args) =>
    switch (value) {
    | Some(match) => Belt.Array.concat(args, [|"MATCH", match|])
    | None => args
    };

  let argsWithCount = (value, args) =>
    switch (value) {
    | Some(count) =>
      Belt.Array.concat(args, [|"COUNT", string_of_int(count)|])
    | None => args
    };

  let argsWithDict = (value, args) => {
    let pairs = Js.Dict.entries(value);
    Belt.Array.reduce(pairs, args, (result, (key, value)) =>
      Belt.Array.concat(result, [|key, value|])
    );
  };

  let rec listToPairs = (~results=[], values) => {
    switch (values) {
    | [key, value, ...rest] =>
      listToPairs(~results=[(key, value), ...results], rest)
    | [] => results
    | _ => results
    };
  };

  [@bs.module] [@bs.new] external make: unit => client = "ioredis";

  [@bs.send] external quit: client => promise = "";

  [@bs.send] external set: (client, array(string)) => promise = "";
  [@bs.send] external get: (client, array(string)) => promise = "";
  [@bs.send] external del: (client, array(string)) => promise = "";
  [@bs.send] external exists: (client, array(string)) => promise = "";
  [@bs.send] external scan: (client, array(string)) => promise = "";

  [@bs.send] external hincrby: (client, array(string)) => promise = "";
  [@bs.send] external hmset: (client, array(string)) => promise = "";
  [@bs.send] external hscan: (client, array(string)) => promise = "";

  [@bs.send] external sadd: (client, array(string)) => promise = "";
  [@bs.send] external scard: (client, array(string)) => promise = "";
  [@bs.send] external sismember: (client, string, string) => promise = "";
};

module Error = {
  type redisError = {
    message: option(string),
    stack: option(Js.Json.t),
  };

  type replyError = {
    message: option(string),
    stack: option(Js.Json.t),
    args: option(array(Js.Json.t)),
    command: option(string),
    code: option(string),
  };

  type parserError = {
    message: option(string),
    stack: option(Js.Json.t),
    buffer: option(string),
    offset: option(int),
  };

  type abortError = {
    message: option(string),
    stack: option(Js.Json.t),
    args: option(array(Js.Json.t)),
    command: option(string),
  };

  type interruptError = {
    message: option(string),
    stack: option(Js.Json.t),
    args: option(array(Js.Json.t)),
    command: option(string),
    origin: option(Js.Exn.t),
  };

  // Attaches the name so we can find errors we missed
  type unknownError = {
    name: option(string),
    message: option(string),
    stack: option(Js.Json.t),
  };
  type t =
    | RedisError(redisError)
    | ReplyError(replyError)
    | ParserError(parserError)
    | AbortError(abortError)
    | InterruptError(interruptError)
    | UnknownError(unknownError);

  let classify = error => {
    Internal.(
      switch (JsError.nameGet(error)) {
      | Some("RedisError") =>
        RedisError({
          message: JsError.messageGet(error),
          stack: JsError.stackGet(error),
        })
      | Some("ReplyError") =>
        ReplyError({
          message: JsError.messageGet(error),
          stack: JsError.stackGet(error),
          args: JsError.argsGet(error),
          command: JsError.commandGet(error),
          code: JsError.codeGet(error),
        })
      | Some("ParserError") =>
        ParserError({
          message: JsError.messageGet(error),
          stack: JsError.stackGet(error),
          buffer: JsError.bufferGet(error),
          offset: JsError.offsetGet(error),
        })
      | Some("AbortError") =>
        AbortError({
          message: JsError.messageGet(error),
          stack: JsError.stackGet(error),
          args: JsError.argsGet(error),
          command: JsError.commandGet(error),
        })
      | Some("InterruptError") =>
        InterruptError({
          message: JsError.messageGet(error),
          stack: JsError.stackGet(error),
          args: JsError.argsGet(error),
          command: JsError.commandGet(error),
          origin: JsError.originGet(error),
        })
      | _ =>
        UnknownError({
          name: JsError.nameGet(error),
          message: JsError.messageGet(error),
          stack: JsError.stackGet(error),
        })
      }
    );
  };
};

module Promise = {
  include Repromise;
};

module SimpleStringReply = {
  type t =
    | Ok
    | Empty
    | Unknown(string);

  let decode = json => {
    let value = json |> Json.Decode.(optional(string));
    switch (value) {
    | Some("OK") => Ok
    | Some(value) => Unknown(value)
    | None => Empty
    };
  };
};

module BulkStringReply = {
  type t = option(string);

  let decode = Json.Decode.(optional(string));
};

module IntegerReply = {
  type t = int;

  let decode = Json.Decode.int;
};

module BooleanReply = {
  // This is actually something I want that isn't provided by ioredis
  type t = bool;

  let decode = json =>
    switch (Json.Decode.int(json)) {
    | 1 => true
    | 0 => false
    | invalid =>
      failwith(
        "Invalid reply for BooleanReply. Got integer: "
        ++ string_of_int(invalid),
      )
    };
};

module ScanReply = {
  // This isn't a spec'd Reply but it is bolded in the SCAN docs
  // TODO: should this be a tuple, record or variant?
  type t('a) = (Cursor.t, 'a);

  let decode = json => {
    let (cursor, results) =
      json |> Json.Decode.(pair(string, list(string)));

    if (cursor === "0") {
      (Cursor.LastCursor, results);
    } else {
      (Cursor.NextCursor(cursor), results);
    };
  };
};

type t = Internal.client;

// TODO: config options
let make = () => Internal.make();

let quit = client => {
  // TODO: they claim this is always OK
  Internal.quit(client)
  |> Repromise.Rejectable.map(json =>
       Belt.Result.Ok(SimpleStringReply.decode(json))
     )
  |> Repromise.Rejectable.catch(error => {
       let result = Belt.Result.Error(Error.classify(error));
       Promise.resolved(result);
     });
};

let set = (~key, ~value, ~expiration=?, ~existence=?, client) => {
  let args =
    [|key, value|]
    |> Internal.argsWithExpiration(expiration)
    |> Internal.argsWithExistence(existence);
  Internal.set(client, args)
  |> Repromise.Rejectable.map(json =>
       Belt.Result.Ok(SimpleStringReply.decode(json))
     )
  |> Repromise.Rejectable.catch(error => {
       let result = Belt.Result.Error(Error.classify(error));
       Promise.resolved(result);
     });
};

let get = (~key, client) => {
  let args = [|key|];
  Internal.get(client, args)
  |> Repromise.Rejectable.map(json =>
       Belt.Result.Ok(BulkStringReply.decode(json))
     )
  |> Repromise.Rejectable.catch(error => {
       let result = Belt.Result.Error(Error.classify(error));
       Promise.resolved(result);
     });
};

let del = (~keys, client) => {
  let args = Belt.List.toArray(keys);

  Internal.del(client, args)
  |> Repromise.Rejectable.map(json =>
       Belt.Result.Ok(IntegerReply.decode(json))
     )
  |> Repromise.Rejectable.catch(error => {
       let result = Belt.Result.Error(Error.classify(error));
       Promise.resolved(result);
     });
};

// This doesn't allow multiple keys because I don't like that API
let exists = (~key, client) => {
  let args = [|key|];
  Internal.exists(client, args)
  |> Repromise.Rejectable.map(json =>
       Belt.Result.Ok(BooleanReply.decode(json))
     )
  |> Repromise.Rejectable.catch(error => {
       let result = Belt.Result.Error(Error.classify(error));
       Promise.resolved(result);
     });
};

let scan = (~cursor, ~match=?, ~count=?, client) => {
  let args =
    [||]
    |> Internal.argsWithCursor(cursor)
    |> Internal.argsWithMatch(match)
    |> Internal.argsWithCount(count);
  Internal.scan(client, args)
  |> Repromise.Rejectable.map(json => {
       let result = ScanReply.decode(json);
       Belt.Result.Ok(result);
     })
  |> Repromise.Rejectable.catch(error => {
       let result = Belt.Result.Error(Error.classify(error));
       Promise.resolved(result);
     });
};

let hincrby = (~key, ~field, ~value, client) => {
  let args = [|key, field, string_of_int(value)|];
  Internal.hincrby(client, args)
  |> Repromise.Rejectable.map(json =>
       Belt.Result.Ok(IntegerReply.decode(json))
     )
  |> Repromise.Rejectable.catch(error => {
       let result = Belt.Result.Error(Error.classify(error));
       Promise.resolved(result);
     });
};

// TODO: I don't like the name `values` because it's actually key/value pairs
// maybe should be ~dict?
let hmset = (~key, ~values, client) => {
  let args = [|key|] |> Internal.argsWithDict(values);

  Internal.hmset(client, args)
  |> Repromise.Rejectable.map(json =>
       Belt.Result.Ok(SimpleStringReply.decode(json))
     )
  |> Repromise.Rejectable.catch(error => {
       let result = Belt.Result.Error(Error.classify(error));
       Promise.resolved(result);
     });
};

let hscan = (~key, ~cursor, ~match=?, ~count=?, client) => {
  let args =
    [|key|]
    |> Internal.argsWithCursor(cursor)
    |> Internal.argsWithMatch(match)
    |> Internal.argsWithCount(count);

  Internal.hscan(client, args)
  |> Repromise.Rejectable.map(json => {
       let result =
         Json.Decode.map(
           ((cursor, keyValues)) =>
             (cursor, Internal.listToPairs(keyValues) |> Js.Dict.fromList),
           ScanReply.decode,
           json,
         );
       Belt.Result.Ok(result);
     })
  |> Repromise.Rejectable.catch(error => {
       let result = Belt.Result.Error(Error.classify(error));
       Promise.resolved(result);
     });
};

// TODO: `values` is actually called `members` in the Redis docs
let sadd = (~key, ~values, client) => {
  // TODO: should this be abstracted into an args normalize function?
  let args = Belt.Array.concat([|key|], Belt.List.toArray(values));

  Internal.sadd(client, args)
  |> Repromise.Rejectable.map(json =>
       Belt.Result.Ok(IntegerReply.decode(json))
     )
  |> Repromise.Rejectable.catch(error => {
       let result = Belt.Result.Error(Error.classify(error));
       Promise.resolved(result);
     });
};

let scard = (~key, client) => {
  let args = [|key|];

  Internal.scard(client, args)
  |> Repromise.Rejectable.map(json =>
       Belt.Result.Ok(IntegerReply.decode(json))
     )
  |> Repromise.Rejectable.catch(error => {
       let result = Belt.Result.Error(Error.classify(error));
       Promise.resolved(result);
     });
};
