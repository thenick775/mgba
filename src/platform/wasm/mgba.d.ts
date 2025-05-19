/// <reference types="emscripten" />

declare namespace mGBA {
  export interface filePaths {
    root: string;
    cheatsPath: string;
    gamePath: string;
    savePath: string;
    saveStatePath: string;
    screenshotsPath: string;
    patchPath: string;
  }

  // see: https://developer.mozilla.org/en-US/docs/Web/API/BaseAudioContext/state
  //      interrupted is a valid property on iOS
  type ExtendedAudioContextState = AudioContextState | 'interrupted';

  export type coreCallbacks = {
    alarmCallback?: (() => void) | null;
    coreCrashedCallback?: (() => void) | null;
    keysReadCallback?: (() => void) | null;
    saveDataUpdatedCallback?: (() => void) | null;
    videoFrameEndedCallback?: (() => void) | null;
    videoFrameStartedCallback?: (() => void) | null;
  };

  export type coreSettings = {
    frameSkip?: number;
    baseFpsTarget?: number;
    rewindBufferCapacity?: number;
    rewindBufferInterval?: number;
    audioSampleRate?: number;
    audioBufferSize?: number;
    allowOpposingDirections?: boolean;
    videoSync?: boolean;
    audioSync?: boolean;
    threadedVideo?: boolean;
    rewindEnable?: boolean;
    timestepSync?: boolean;
    showFpsCounter?: boolean;
  };

  export interface mGBAEmulator extends EmscriptenModule {
    // custom methods from preamble
    autoLoadCheats(): boolean;
    bindKey(bindingName: string, inputName: string): void;
    buttonPress(name: string): void;
    buttonUnpress(name: string): void;
    FSInit(): Promise<void>;
    FSSync(): Promise<void>;
    getFastForwardMultiplier(): number;
    getMainLoopTimingMode(): number;
    getMainLoopTimingValue(): number;
    getSave(): Uint8Array;
    getVolume(): number;
    listRoms(): string[];
    listSaves(): string[];
    loadGame(romPath: string, savePathOverride?: string): boolean;
    loadState(slot: number): boolean;
    pauseAudio(): void;
    pauseGame(): void;
    quickReload(): void;
    quitGame(): void;
    quitMgba(): void;
    resumeAudio(): void;
    resumeGame(): void;
    saveState(slot: number): boolean;
    screenshot(fileName?: string): boolean;
    setFastForwardMultiplier(multiplier: number): void;
    setMainLoopTiming(mode: number, value: number): void;
    setVolume(percent: number): void;
    toggleInput(enabled: boolean): void;
    uploadCheats(file: File, callback?: () => void): void;
    uploadPatch(file: File, callback?: () => void): void;
    uploadRom(file: File, callback?: () => void): void;
    uploadSaveOrSaveState(file: File, callback?: () => void): void;
    addCoreCallbacks(coreCallbacks: coreCallbacks): void;
    toggleRewind(enabled: boolean): void;
    setCoreSettings(coreSettings: CoreSettings): void;
    // custom variables
    version: {
      projectName: string;
      projectVersion: string;
    };
    filePaths(): filePaths;
    gameName?: string;
    saveName?: string;
    // extra exported runtime methods
    FS: typeof FS;
    // SDL2
    SDL2: {
      audio: {
        currentOutputBuffer: AudioBuffer;
        scriptProcessorNode: ScriptProcessorNode;
      };
      audioContext: Omit<AudioContext, 'state'> & {
        readonly state: ExtendedAudioContextState;
      };
    };
  }

  // eslint-disable-next-line import/no-default-export
  export default function mGBA(options: {
    canvas: HTMLCanvasElement;
  }): Promise<mGBAEmulator>;
}

export = mGBA;
