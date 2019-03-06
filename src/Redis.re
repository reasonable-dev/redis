type cursor;

external cursor: int => cursor = "%identity";

type existence =
  | XX
  | NX;

type expiration =
  | EX(int)
  | PX(int);

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

  [@bs.module] [@bs.new] external make: unit => client = "ioredis";

  [@bs.send] external quit: client => promise = "";

  [@bs.send] external set: (client, array(string)) => promise = "";
  [@bs.send] external get: (client, array(string)) => promise = "";
  [@bs.send] external del: (client, array(string)) => promise = "";

  [@bs.send] external hincrby: (client, array(string)) => promise = "";
  [@bs.send] external hscan: (client, string, cursor) => promise = "";
  [@bs.send] external hmset: (client, string, Js.Json.t) => promise = "";

  [@bs.send] external sadd: (client, string, string) => promise = "";
  [@bs.send] external scard: (client, string) => promise = "";
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
        // Js.log(JsError.messageGet(error));
        // Js.log(JsError.commandGet(argsGet(error));
        // Js.log(JsError.commandGet(error));
        // Js.log(JsError.codeGet(error));

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
  Internal.del(client, keys)
  |> Repromise.Rejectable.map(json =>
       Belt.Result.Ok(IntegerReply.decode(json))
     )
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
