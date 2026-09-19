(() => {
  "use strict";

  const TICK = 1 / 60;
  const SLOTS = 36;

  const Outcome = { Playing: 0, Win: 1, DeathMelee: 2, DeathRiver: 3 };
  const Feint = { Approach: 0, Windup: 1, Lunge: 2 };
  const Mode = { Cover: 0, Select: 1, Playing: 2, Clear: 3, AllClear: 4, Dead: 5 };

  const I18N = {
    "game.title": ["暴虎冯河", "Bao Hu Ping He"],
    "cover.tagline": ["有勇无度", "Courage without measure"],
    "cover.prompt": ["按任意键 / 点击继续", "Press any key / tap to continue"],
    "select.level": ["选择关卡", "Select stage"],
    "hud.level": ["关卡", "Level"],
    "hud.rules": [
      "空格 / 按住按钮蓄力，松开冲刺 · 撑过计时过关 · 红区或落水则死",
      "Space / hold to charge, release to dash · Survive the timer · Red zone or river kills",
    ],
    "level.1.title": ["试水", "Wading"],
    "level.2.title": ["逼择", "Hard Choice"],
    "level.3.title": ["诡步", "Strange Steps"],
    "level.4.title": ["渡口", "Ferry"],
    "level.5.title": ["狂者", "Mad One"],
    "level.6.title": ["妄执", "Delusion"],
    "end.melee": ["贴身死 · 有勇无距", "Mauled · Courage without distance"],
    "end.river": ["落水死 · 退亦险境", "Drowned · Retreat into peril"],
    "end.level_clear": ["本关通过", "Stage cleared"],
    "end.all_clear": ["六关通关 · 勇而有度", "All clear · Brave, yet measured"],
    "btn.next": ["下一关 N", "Next N"],
    "btn.retry": ["重玩 R", "Retry R"],
    "btn.back": ["选关 Q", "Stages Q"],
    "btn.hold": ["按住蓄力 · Hold", "Hold to charge"],
    "play.keys": [
      "空格 / 按住按钮蓄力，松开冲刺 · 撑过计时过关 · 红区或落水则死",
      "Space / hold to charge, release to dash · Survive the timer · Red zone or river kills",
    ],
  };

  let locale = "zh";
  function tr(key) {
    const row = I18N[key];
    if (!row) return key;
    return locale === "en" ? row[1] : row[0];
  }

  function defaults() {
    return {
      worldMinX: 0,
      riverX: 16,
      riverWidth: 3,
      playerStartX: 8,
      tigerStartX: 4,
      meleeDist: 1.35,
      surviveSec: 12,
      chargeFullSec: 0.83,
      tigerSpeed: 2.16,
      walkSpeed: 1.85,
      chargeWalkFactor: 0.5,
      chargeTigerSpeedFactor: 1.15,
      dashTigerSpeedFactor: 0.9,
      dashRange: 4.2,
      knockbackRange: 3.85,
      recoilRange: 2.8,
      recoilBaselineCharge: 0.75,
      recoilFullFactor: 1.4,
      hitRadius: 1.7,
      impactAnimSec: 0.28,
      bankSlipFactor: 0,
      feintEnabled: false,
      feintInterval: 4,
      feintWindupSec: 0.55,
      feintLungeSec: 0.4,
      feintRetreatSpeed: 0.35,
      feintLungeFactor: 2.4,
      tideEnabled: false,
      tideSpeed: 0.22,
      tideMinRiverX: 0,
      tideExtraRiverSlots: 4,
      boatEnabled: false,
      boatOffset: 0.9,
      boatHalfWidth: 0.65,
      madSlipEnabled: false,
      madSlipOnPeak: true,
      madSlipHoldSec: 0,
      madChargeWalkBoost: 1.15,
      puppetTigerEnabled: false,
      puppetLungeRange: 3.2,
      puppetLungeMin: 0.85,
      puppetChargeRushSpeed: 1.85,
    };
  }

  function level(id, titleKey, patch, riverSlots) {
    return { id, titleKey, game: Object.assign(defaults(), patch), riverSlots };
  }

  const LEVELS = [
    level("tutorial", "level.1.title", {
      surviveSec: 11, tigerSpeed: 1.55, walkSpeed: 1.7, riverX: 16.5,
      playerStartX: 8.5, tigerStartX: 3.2, meleeDist: 1.25, recoilFullFactor: 1.28,
    }, 6),
    level("pressure", "level.2.title", {
      surviveSec: 12, tigerSpeed: 2.3, walkSpeed: 1.88, riverX: 13.8,
      playerStartX: 7.2, tigerStartX: 3.2, meleeDist: 1.4, bankSlipFactor: 0.2,
      recoilRange: 2.89, recoilFullFactor: 1.49, chargeTigerSpeedFactor: 1.23,
    }, 8),
    level("feint_tide", "level.3.title", {
      surviveSec: 14, tigerSpeed: 2.05, walkSpeed: 1.9, riverX: 14.8,
      playerStartX: 7.6, tigerStartX: 3.4, meleeDist: 1.38, bankSlipFactor: 0.14,
      recoilFullFactor: 1.42, feintEnabled: true, feintInterval: 2.8,
      feintWindupSec: 0.48, feintLungeSec: 0.42, feintRetreatSpeed: 0.35,
      feintLungeFactor: 2.9, tideEnabled: true, tideSpeed: 0.7,
      tideMinRiverX: 10.5, tideExtraRiverSlots: 6,
    }, 6),
    level("ferry_secret", "level.4.title", {
      surviveSec: 16, tigerSpeed: 2.35, walkSpeed: 2.0, riverX: 13.5,
      playerStartX: 7.0, tigerStartX: 3.4, meleeDist: 1.42, bankSlipFactor: 0.28,
      recoilRange: 3.35, recoilFullFactor: 1.55, chargeTigerSpeedFactor: 1.28,
      feintEnabled: true, feintInterval: 2.6, feintWindupSec: 0.42,
      feintLungeSec: 0.45, feintRetreatSpeed: 0.5, feintLungeFactor: 3.0,
      tideEnabled: true, tideSpeed: 0.75, tideMinRiverX: 10.0,
      tideExtraRiverSlots: 6, boatEnabled: true, boatOffset: 0.75, boatHalfWidth: 0.32,
    }, 6),
    level("mad_slip", "level.5.title", {
      surviveSec: 15, tigerSpeed: 2.4, walkSpeed: 1.95, riverX: 13.8,
      playerStartX: 7.2, tigerStartX: 3.4, meleeDist: 1.4, bankSlipFactor: 0.22,
      recoilRange: 3.0, recoilFullFactor: 1.5, chargeFullSec: 0.5,
      chargeTigerSpeedFactor: 1.3, feintEnabled: true, feintInterval: 2.7,
      feintWindupSec: 0.42, feintLungeSec: 0.4, feintRetreatSpeed: 0.4,
      feintLungeFactor: 2.85, madSlipEnabled: true, madSlipOnPeak: true,
      madChargeWalkBoost: 1.35,
    }, 6),
    level("tiger_puppet", "level.6.title", {
      surviveSec: 13, tigerSpeed: 0.42, walkSpeed: 1.75, riverX: 15.0,
      playerStartX: 8.8, tigerStartX: 3.0, meleeDist: 1.38, chargeFullSec: 0.9,
      puppetTigerEnabled: true, puppetLungeRange: 3.0, puppetLungeMin: 0.45,
      puppetChargeRushSpeed: 1.55,
    }, 6),
  ];

  function clamp(v, a, b) { return Math.max(a, Math.min(b, v)); }

  function makeState(cfg) {
    const s = {
      playerX: cfg.playerStartX,
      tigerX: cfg.tigerStartX,
      riverX: cfg.riverX,
      tideRising: true,
      boatX: cfg.riverX + cfg.boatOffset,
      timeLeft: cfg.surviveSec,
      charge01: 0,
      chargeRising: true,
      charging: false,
      spaceHeld: false,
      chargeHoldSec: 0,
      outcome: Outcome.Playing,
      animSec: 0,
      animDur: 0,
      animPlayerFrom: cfg.playerStartX,
      animPlayerMid: cfg.playerStartX,
      animPlayerTo: cfg.playerStartX,
      animTigerFrom: cfg.tigerStartX,
      animTigerTo: cfg.tigerStartX,
      impactHit: false,
      feintPhase: Feint.Approach,
      feintTimer: cfg.feintEnabled ? cfg.feintInterval : 0,
    };
    enforce(s, cfg);
    return s;
  }

  function enforce(s, cfg) {
    if (s.tigerX < cfg.worldMinX) s.tigerX = cfg.worldMinX;
    if (s.playerX <= s.tigerX) s.playerX = s.tigerX + 0.01;
  }

  function onBoat(s, cfg) {
    return cfg.boatEnabled && Math.abs(s.playerX - s.boatX) <= cfg.boatHalfWidth + 1e-9;
  }

  function syncBoat(s, cfg) {
    if (cfg.boatEnabled) s.boatX = s.riverX + cfg.boatOffset;
  }

  function checkTerminal(s, cfg) {
    if (s.outcome !== Outcome.Playing) return;
    if (s.playerX >= s.riverX) {
      s.outcome = Outcome.DeathRiver;
      // 保留 animSec，让反冲飞行动画播完再结算
      return;
    }
    if (s.playerX - s.tigerX <= cfg.meleeDist + 1e-9) {
      s.outcome = Outcome.DeathMelee;
      s.playerX = s.tigerX + cfg.meleeDist;
      return;
    }
    if (s.timeLeft <= 0) {
      s.timeLeft = 0;
      s.outcome = Outcome.Win;
    }
  }

  function beginAnim(s, cfg, fromP, midP, toP, fromT, toT, hit) {
    // 两段动画略长于逻辑冲击感，但仍保持干脆（勿抬到 0.5s+）
    s.animDur = Math.max(0.18, Math.min(cfg.impactAnimSec * 1.15, 0.34));
    s.animSec = s.animDur;
    s.animPlayerFrom = fromP;
    s.animPlayerMid = midP;
    s.animPlayerTo = toP;
    s.animTigerFrom = fromT;
    s.animTigerTo = toT;
    s.impactHit = hit;
  }

  function recoilMul(p, cfg) {
    const baseAt = clamp(cfg.recoilBaselineCharge, 0.05, 0.99);
    p = clamp(p, 0, 1);
    if (p <= baseAt) return p / baseAt;
    const t = (p - baseAt) / (1 - baseAt);
    return 1 + t * (cfg.recoilFullFactor - 1);
  }

  function tryDash(s, cfg) {
    if (s.outcome !== Outcome.Playing) return;
    const p = clamp(s.charge01, 0, 1);
    s.charge01 = 0;
    s.charging = false;
    s.chargeHoldSec = 0;
    if (p <= 0) return;

    const playerBefore = s.playerX;
    const tigerBefore = s.tigerX;

    if (cfg.puppetTigerEnabled) {
      const lunge = Math.max(cfg.puppetLungeMin, p * cfg.puppetLungeRange);
      s.tigerX += lunge;
      const contact = s.playerX - cfg.meleeDist;
      let hit = false;
      if (s.tigerX >= contact - 1e-9) {
        s.tigerX = contact;
        hit = true;
      }
      if (s.tigerX < cfg.worldMinX) s.tigerX = cfg.worldMinX;
      beginAnim(s, cfg, playerBefore, playerBefore, s.playerX, tigerBefore, s.tigerX, hit);
      checkTerminal(s, cfg);
      return;
    }

    const contactX = s.tigerX + cfg.hitRadius;
    const dashDist = p * cfg.dashRange;
    const projected = s.playerX - dashDist;
    // 落到击打半径内（含已在半径内）一律算撞上，避免“看着撞了却贴死”
    const hitForgive = 0.15;
    const willHit = projected <= contactX + hitForgive;

    if (willHit) {
      // 先撞上，再反冲：动画 mid=接触点，to=反冲落点
      const recoil = recoilMul(p, cfg) * cfg.recoilRange;
      s.playerX = contactX;
      s.tigerX -= p * cfg.knockbackRange;
      if (s.tigerX < cfg.worldMinX) s.tigerX = cfg.worldMinX;
      const mid = contactX;
      s.playerX += recoil;
      if (cfg.bankSlipFactor > 0) s.playerX += recoil * cfg.bankSlipFactor;
      enforce(s, cfg);
      // 命中后保证离开贴身区，防止反冲过小立刻判贴死
      const minGap = cfg.meleeDist + 0.08;
      if (s.playerX < s.tigerX + minGap) s.playerX = s.tigerX + minGap;
      beginAnim(s, cfg, playerBefore, mid, s.playerX, tigerBefore, s.tigerX, true);
    } else {
      s.playerX = projected;
      enforce(s, cfg);
      // 未命中时动画不要冲进红区尖端，减少“假撞”观感
      const mid = playerBefore + (s.playerX - playerBefore) * 0.5;
      beginAnim(s, cfg, playerBefore, mid, s.playerX, tigerBefore, s.tigerX, false);
    }

    syncBoat(s, cfg);
    if (onBoat(s, cfg)) {
      s.outcome = Outcome.Win;
      return;
    }
    checkTerminal(s, cfg);
  }

  function setCharging(s, cfg, down) {
    if (s.outcome !== Outcome.Playing) return;
    s.spaceHeld = down;
    if (down) {
      if (!s.charging) {
        s.charging = true;
        s.charge01 = 0;
        s.chargeRising = true;
        s.chargeHoldSec = 0;
      }
      return;
    }
    if (s.charging || s.charge01 > 0) tryDash(s, cfg);
  }

  function tick(s, cfg, dt) {
    if (s.outcome !== Outcome.Playing) return;
    if (dt < 0) dt = 0;
    s.timeLeft -= dt;
    if (s.animSec > 0) {
      s.animSec = Math.max(0, s.animSec - dt);
      // 冲刺动画期间仍涨退潮，避免河面停住再跳变
      if (cfg.tideEnabled) {
        const maxX = cfg.riverX;
        const minX = cfg.tideMinRiverX;
        if (s.tideRising) {
          s.riverX -= cfg.tideSpeed * dt;
          if (s.riverX <= minX) {
            s.riverX = minX;
            s.tideRising = false;
          }
        } else {
          s.riverX += cfg.tideSpeed * dt;
          if (s.riverX >= maxX) {
            s.riverX = maxX;
            s.tideRising = true;
          }
        }
        syncBoat(s, cfg);
      }
      return;
    }
    s.charging = s.spaceHeld;

    if (s.spaceHeld) {
      const rate = dt / Math.max(0.05, cfg.chargeFullSec);
      const before = s.charge01;
      const wasRising = s.chargeRising;
      if (s.chargeRising) {
        s.charge01 += rate;
        if (s.charge01 >= 1) {
          s.charge01 = 1;
          s.chargeRising = false;
        }
      } else {
        s.charge01 -= rate;
        if (s.charge01 <= 0) {
          s.charge01 = 0;
          s.chargeRising = true;
        }
      }
      let slip = false;
      if (cfg.madSlipEnabled) {
        if (cfg.madSlipOnPeak && wasRising && before < 1 && s.charge01 >= 1 - 1e-12) slip = true;
        if (cfg.madSlipHoldSec > 0) {
          s.chargeHoldSec += dt;
          if (s.chargeHoldSec >= cfg.madSlipHoldSec && s.charge01 > 0) slip = true;
        }
      }
      if (slip) {
        tryDash(s, cfg);
        if (s.outcome === Outcome.Playing && s.spaceHeld) s.chargeRising = true;
        if (s.animSec > 0 || s.outcome !== Outcome.Playing) return;
      }
    } else {
      s.chargeHoldSec = 0;
    }

    if (cfg.tideEnabled) {
      const maxX = cfg.riverX;
      const minX = cfg.tideMinRiverX;
      if (s.tideRising) {
        s.riverX -= cfg.tideSpeed * dt;
        if (s.riverX <= minX) {
          s.riverX = minX;
          s.tideRising = false;
        }
      } else {
        s.riverX += cfg.tideSpeed * dt;
        if (s.riverX >= maxX) {
          s.riverX = maxX;
          s.tideRising = true;
        }
      }
    }
    syncBoat(s, cfg);

    if (cfg.feintEnabled) {
      s.feintTimer -= dt;
      if (s.feintTimer <= 0) {
        if (s.feintPhase === Feint.Approach) {
          s.feintPhase = Feint.Windup;
          s.feintTimer = cfg.feintWindupSec;
        } else if (s.feintPhase === Feint.Windup) {
          s.feintPhase = Feint.Lunge;
          s.feintTimer = cfg.feintLungeSec;
        } else {
          s.feintPhase = Feint.Approach;
          s.feintTimer = cfg.feintInterval;
        }
      }
    }

    if (cfg.puppetTigerEnabled) {
      if (s.spaceHeld) s.playerX -= cfg.puppetChargeRushSpeed * dt;
      else s.playerX += cfg.walkSpeed * dt;
    } else {
      let speed = s.charging ? cfg.walkSpeed * cfg.chargeWalkFactor : cfg.walkSpeed;
      if (cfg.madSlipEnabled && s.charging) speed *= cfg.madChargeWalkBoost;
      s.playerX += speed * dt;
    }

    let tigerSpd = cfg.tigerSpeed;
    if (cfg.puppetTigerEnabled) {
      tigerSpd = cfg.tigerSpeed;
    } else if (cfg.feintEnabled) {
      if (s.feintPhase === Feint.Windup) tigerSpd = -cfg.feintRetreatSpeed;
      else if (s.feintPhase === Feint.Lunge) tigerSpd = cfg.tigerSpeed * cfg.feintLungeFactor;
      else if (s.charging) tigerSpd *= cfg.chargeTigerSpeedFactor;
    } else if (s.charging) {
      tigerSpd *= cfg.chargeTigerSpeedFactor;
    }

    s.tigerX += tigerSpd * dt;
    if (s.tigerX < cfg.worldMinX) s.tigerX = cfg.worldMinX;
    enforce(s, cfg);
    checkTerminal(s, cfg);
  }

  function displayPlayerX(s) {
    if (s.animSec <= 0 || s.animDur <= 0) return s.playerX;
    const t = 1 - s.animSec / s.animDur;
    const mid = s.animPlayerMid;
    // 前段更快撞上，后段快速弹回
    if (t < 0.32) {
      const u = t / 0.32;
      const e = u * u * (3 - 2 * u); // smoothstep，起势快
      return s.animPlayerFrom + (mid - s.animPlayerFrom) * e;
    }
    const u = (t - 0.32) / 0.68;
    const e = 1 - Math.pow(1 - u, 3);
    return mid + (s.animPlayerTo - mid) * e;
  }

  function displayTigerX(s) {
    if (s.animSec <= 0 || s.animDur <= 0) return s.tigerX;
    const t = 1 - s.animSec / s.animDur;
    if (t < 0.32) return s.animTigerFrom;
    const u = (t - 0.32) / 0.68;
    const e = 1 - Math.pow(1 - u, 2.2);
    return s.animTigerFrom + (s.animTigerTo - s.animTigerFrom) * e;
  }

  // —— UI ——
  const $ = (id) => document.getElementById(id);
  const el = {
    title: $("title"),
    tagline: $("tagline"),
    cover: $("screen-cover"),
    select: $("screen-select"),
    play: $("screen-play"),
    coverStrip: $("cover-strip"),
    coverPrompt: $("cover-prompt"),
    selectLabel: $("select-label"),
    levelList: $("level-list"),
    levelLine: $("level-line"),
    timeLine: $("time-line"),
    chargeFill: $("charge-fill"),
    chargePct: $("charge-pct"),
    arena: $("arena"),
    banner: $("banner"),
    holdBtn: $("hold-btn"),
    menuBtns: $("menu-btns"),
    btnNext: $("btn-next"),
    btnRetry: $("btn-retry"),
    btnBack: $("btn-back"),
    playKeys: $("play-keys"),
    btnZh: $("btn-zh"),
    btnEn: $("btn-en"),
  };

  let mode = Mode.Cover;
  let levelIndex = 0;
  let levelCfg = LEVELS[0];
  let state = makeState(levelCfg.game);
  let spaceDown = false;
  let acc = 0;
  let lastTs = 0;
  let menuArmed = false;

  function showScreen(which) {
    el.cover.classList.toggle("active", which === "cover");
    el.select.classList.toggle("active", which === "select");
    el.play.classList.toggle("active", which === "play");
  }

  function refreshChrome() {
    el.title.textContent = tr("game.title");
    el.tagline.textContent = tr("cover.tagline");
    el.coverPrompt.textContent = tr("cover.prompt");
    el.selectLabel.textContent = tr("select.level");
    el.holdBtn.textContent = tr("btn.hold");
    el.btnNext.textContent = tr("btn.next");
    el.btnRetry.textContent = tr("btn.retry");
    el.btnBack.textContent = tr("btn.back");
    el.playKeys.textContent = tr("play.keys");
    buildLevelList();
    if (mode === Mode.Playing || mode === Mode.Clear || mode === Mode.Dead || mode === Mode.AllClear) {
      renderHud();
    }
  }

  function buildLevelList() {
    el.levelList.innerHTML = "";
    LEVELS.forEach((lv, i) => {
      const b = document.createElement("button");
      b.type = "button";
      b.innerHTML = `<span class="num">${i + 1}</span>${tr(lv.titleKey)}`;
      b.addEventListener("click", () => startLevel(i));
      el.levelList.appendChild(b);
    });
  }

  function paintCoverStrip() {
    el.coverStrip.innerHTML = "";
    const arena = document.createElement("div");
    arena.className = "arena";
    arena.innerHTML = `
      <div class="lane"></div>
      <div class="zone-threat" style="left:28%;width:22%"></div>
      <div class="zone-river" style="width:28%"></div>
      <div class="token tiger" style="left:22%">虎</div>
      <div class="token player" style="left:52%">人</div>
    `;
    el.coverStrip.appendChild(arena);
  }

  function worldToLandPct(x, minX, riverX, landPct) {
    const span = Math.max(1e-6, riverX - minX);
    const t = clamp((x - minX) / span, 0, 1);
    // 陆地占 arena 左侧 landPct%，贴齐河岸，不再死顶在 70% 留空隙
    const pad = 2.2;
    return pad + t * Math.max(0, landPct - pad - 1.2);
  }

  function worldToArenaPct(x, minX, riverX, landPct, riverPct, riverWidth) {
    if (x < riverX - 1e-9) {
      return worldToLandPct(x, minX, riverX, landPct);
    }
    const into = Math.max(0, x - riverX);
    const depth = clamp(into / Math.max(0.45, riverWidth * 0.55), 0, 1);
    // 从河岸连续滑入，禁止最小深度“瞬移吸进”
    return landPct + depth * Math.max(3, riverPct * 0.62);
  }

  let arenaNodes = null;
  let arenaW = 1;
  let hudCache = { time: "", charge: -1, river: -1, threatL: -1, threatW: -1, hot: null, boatOn: null, hitOn: null };

  function measureArena() {
    const w = el.arena.clientWidth;
    if (w > 0) arenaW = w;
  }

  function setPctStyle(node, prop, pct) {
    const v = Math.round(pct * 40) / 40; // 0.025% 精度，避免无意义重写
    const key = "_" + prop;
    if (node[key] === v) return;
    node[key] = v;
    node.style[prop] = `${v}%`;
  }

  function setTokenPct(node, pct) {
    const v = Math.round(pct * 40) / 40;
    if (node._pct === v) return;
    node._pct = v;
    node.style.transform = `translate3d(${(v / 100) * arenaW}px, -50%, 0)`;
  }

  function ensureArena() {
    if (arenaNodes) return arenaNodes;
    el.arena.innerHTML = `
      <div class="lane" data-n="lane"></div>
      <div class="zone-threat" data-n="threat"></div>
      <div class="zone-river" data-n="river"></div>
      <div class="token boat hidden" data-n="boat">舟</div>
      <div class="token tiger" data-n="tiger">虎</div>
      <div class="token player" data-n="player">人</div>
      <div class="token hit hidden" data-n="hit">撞</div>
    `;
    arenaNodes = {
      lane: el.arena.querySelector('[data-n="lane"]'),
      threat: el.arena.querySelector('[data-n="threat"]'),
      river: el.arena.querySelector('[data-n="river"]'),
      boat: el.arena.querySelector('[data-n="boat"]'),
      tiger: el.arena.querySelector('[data-n="tiger"]'),
      player: el.arena.querySelector('[data-n="player"]'),
      hit: el.arena.querySelector('[data-n="hit"]'),
    };
    hudCache = { time: "", charge: -1, river: -1, threatL: -1, threatW: -1, hot: null, boatOn: null, hitOn: null };
    measureArena();
    return arenaNodes;
  }

  function renderArena() {
    const n = ensureArena();
    const cfg = levelCfg.game;
    const s = state;
    let riverSlots = clamp(levelCfg.riverSlots, 2, SLOTS - 8);
    let dynamicRiver = riverSlots;
    if (cfg.tideEnabled) {
      const span = Math.max(0.01, cfg.riverX - cfg.tideMinRiverX);
      const progress = clamp((cfg.riverX - s.riverX) / span, 0, 1);
      // 连续插值，不要 Math.round，否则涨潮会一格一格卡
      dynamicRiver = clamp(
        riverSlots + progress * cfg.tideExtraRiverSlots,
        riverSlots,
        SLOTS - 6
      );
    }
    const riverPct = (dynamicRiver / SLOTS) * 100;
    const landPct = 100 - riverPct;
    const trackMax = s.riverX;

    const lethal =
      (s.outcome === Outcome.DeathMelee && s.animSec <= 0) ||
      (s.outcome === Outcome.Playing && s.playerX - s.tigerX <= cfg.meleeDist + 1e-9);
    let px = s.animSec > 0 ? displayPlayerX(s) : s.playerX;
    let tx = s.animSec > 0 ? displayTigerX(s) : s.tigerX;
    if (lethal && s.animSec <= 0) {
      px = s.playerX;
      tx = s.tigerX;
    }
    if (px < tx + 0.01) px = tx + 0.01;

    if (s.outcome === Outcome.DeathRiver && s.animSec <= 0) {
      px = Math.max(px, trackMax + Math.min(0.55, cfg.riverWidth * 0.25));
    }

    const tigerPct = worldToLandPct(tx, cfg.worldMinX, trackMax, landPct);
    let playerPct;
    if (cfg.boatEnabled && Math.abs(px - s.boatX) <= cfg.boatHalfWidth + 1e-9 && px >= trackMax - 1e-9) {
      const into = Math.max(0, s.boatX - s.riverX);
      playerPct = landPct + (into / Math.max(0.5, cfg.boatOffset + 0.5)) * riverPct * 0.45;
    } else {
      playerPct = worldToArenaPct(px, cfg.worldMinX, trackMax, landPct, riverPct, cfg.riverWidth);
    }

    const threatEnd = worldToLandPct(tx + cfg.meleeDist, cfg.worldMinX, trackMax, landPct);
    const threatLeft = tigerPct;
    const threatWidth = Math.max(0, Math.min(threatEnd, landPct) - threatLeft);
    const windup = cfg.feintEnabled && s.feintPhase === Feint.Windup;
    const mad = cfg.madSlipEnabled;

    if (hudCache.river !== riverPct) {
      hudCache.river = riverPct;
      n.lane.style.right = `${riverPct}%`;
      n.river.style.width = `${riverPct}%`;
    }
    setPctStyle(n.threat, "left", threatLeft);
    setPctStyle(n.threat, "width", threatWidth);
    if (hudCache.hot !== windup) {
      hudCache.hot = windup;
      n.threat.classList.toggle("hot", windup);
    }
    setTokenPct(n.tiger, tigerPct);
    setTokenPct(n.player, playerPct);
    const wantMad = !!mad;
    if (n.player.dataset.mad !== String(wantMad)) {
      n.player.dataset.mad = String(wantMad);
      n.player.classList.toggle("mad", wantMad);
      n.player.textContent = wantMad ? "疯" : "人";
    }

    if (cfg.boatEnabled) {
      const into = Math.max(0, s.boatX - s.riverX);
      const boatPct = landPct + clamp(
        (into / Math.max(0.5, cfg.boatOffset + cfg.boatHalfWidth)) * riverPct * 0.5,
        4,
        riverPct - 4
      );
      if (hudCache.boatOn !== true) {
        hudCache.boatOn = true;
        n.boat.classList.remove("hidden");
      }
      setTokenPct(n.boat, boatPct);
    } else if (hudCache.boatOn !== false) {
      hudCache.boatOn = false;
      n.boat.classList.add("hidden");
    }

    const showHit = s.animSec > 0 && s.impactHit && !lethal;
    if (showHit) {
      const hitPct = (tigerPct + Math.min(playerPct, landPct)) / 2;
      if (hudCache.hitOn !== true) {
        hudCache.hitOn = true;
        n.hit.classList.remove("hidden");
      }
      setTokenPct(n.hit, hitPct);
    } else if (hudCache.hitOn !== false) {
      hudCache.hitOn = false;
      n.hit.classList.add("hidden");
    }
  }

  function renderHud() {
    const lv = levelCfg;
    const levelText = `${tr("hud.level")} ${levelIndex + 1}/${LEVELS.length}  ${tr(lv.titleKey)}`;
    if (el.levelLine.textContent !== levelText) el.levelLine.textContent = levelText;
    const timeText = `${Math.max(0, state.timeLeft) | 0}s`;
    if (hudCache.time !== timeText) {
      hudCache.time = timeText;
      el.timeLine.textContent = timeText;
    }
    const pct = Math.round(clamp(state.charge01, 0, 1) * 100);
    if (hudCache.charge !== pct) {
      hudCache.charge = pct;
      el.chargeFill.style.transform = `scaleX(${pct / 100})`;
      el.chargePct.textContent = `${pct}%`;
    }
    renderArena();
  }

  function armMenu() {
    menuArmed = false;
    setTimeout(() => { menuArmed = true; }, 200);
  }

  function showEndUI() {
    el.menuBtns.classList.remove("hidden");
    el.holdBtn.classList.add("hidden");
    el.banner.classList.remove("hidden");
    if (mode === Mode.Clear) {
      el.banner.className = "banner win";
      el.banner.textContent = tr("end.level_clear");
      el.btnNext.classList.remove("hidden");
    } else if (mode === Mode.AllClear) {
      el.banner.className = "banner win";
      el.banner.textContent = tr("end.all_clear");
      el.btnNext.classList.add("hidden");
    } else {
      el.banner.className = "banner dead";
      el.banner.textContent =
        state.outcome === Outcome.DeathMelee ? tr("end.melee") : tr("end.river");
      el.btnNext.classList.add("hidden");
    }
  }

  function hideEndUI() {
    el.menuBtns.classList.add("hidden");
    el.holdBtn.classList.remove("hidden");
    el.banner.classList.add("hidden");
  }

  function startLevel(i) {
    levelIndex = clamp(i, 0, LEVELS.length - 1);
    levelCfg = LEVELS[levelIndex];
    state = makeState(levelCfg.game);
    spaceDown = false;
    el.holdBtn.classList.remove("pressed");
    mode = Mode.Playing;
    hideEndUI();
    showScreen("play");
    arenaNodes = null;
    renderHud();
    measureArena();
    // 重建后按当前宽度重摆一次，避免首帧 token 用旧宽度
    if (arenaNodes) {
      arenaNodes.tiger._pct = undefined;
      arenaNodes.player._pct = undefined;
      arenaNodes.boat._pct = undefined;
      arenaNodes.hit._pct = undefined;
      renderArena();
    }
  }

  function backToSelect() {
    mode = Mode.Select;
    spaceDown = false;
    showScreen("select");
    refreshChrome();
  }

  function setSpace(down) {
    if (mode !== Mode.Playing) return;
    if (down === spaceDown) return;
    spaceDown = down;
    el.holdBtn.classList.toggle("pressed", down);
    setCharging(state, levelCfg.game, down);
  }

  function onOutcome() {
    if (state.outcome === Outcome.Win) {
      mode = levelIndex + 1 >= LEVELS.length ? Mode.AllClear : Mode.Clear;
      armMenu();
      showEndUI();
    } else if (state.outcome === Outcome.DeathMelee || state.outcome === Outcome.DeathRiver) {
      mode = Mode.Dead;
      armMenu();
      showEndUI();
    }
  }

  function frame(ts) {
    if (!lastTs) lastTs = ts;
    let dt = (ts - lastTs) / 1000;
    lastTs = ts;
    if (dt > 0.05) dt = 0.05;
    acc += dt;
    while (acc >= TICK) {
      if (mode === Mode.Playing) {
        state.spaceHeld = spaceDown;
        tick(state, levelCfg.game, TICK);
        // 结局已定但仍在播反冲动画：只推进动画，不立刻弹结算
        if (state.outcome !== Outcome.Playing) {
          if (state.animSec > 0) {
            state.animSec = Math.max(0, state.animSec - TICK);
          } else {
            onOutcome();
          }
        }
      } else if (
        (mode === Mode.Clear || mode === Mode.Dead || mode === Mode.AllClear) &&
        state.animSec > 0
      ) {
        state.animSec = Math.max(0, state.animSec - TICK);
      }
      acc -= TICK;
    }
    if (mode === Mode.Playing || mode === Mode.Clear || mode === Mode.Dead || mode === Mode.AllClear) {
      renderHud();
    }
    requestAnimationFrame(frame);
  }

  // input
  window.addEventListener("keydown", (e) => {
    if (mode === Mode.Cover) {
      mode = Mode.Select;
      showScreen("select");
      refreshChrome();
      return;
    }
    if (mode === Mode.Select) {
      if (e.key === "z" || e.key === "Z") {
        locale = "zh";
        refreshChrome();
      } else if (e.key === "e" || e.key === "E") {
        locale = "en";
        refreshChrome();
      } else if (e.key >= "1" && e.key <= "6") {
        startLevel(Number(e.key) - 1);
      }
      return;
    }
    if (e.code === "Space") {
      e.preventDefault();
      if (mode === Mode.Playing) setSpace(true);
      return;
    }
    if (!menuArmed) return;
    if (e.key === "r" || e.key === "R") {
      if (mode === Mode.Clear || mode === Mode.Dead) startLevel(levelIndex);
      if (mode === Mode.AllClear) startLevel(0);
    } else if (e.key === "n" || e.key === "N" || e.key === "Enter") {
      if (mode === Mode.Clear) startLevel(levelIndex + 1);
    } else if (e.key === "q" || e.key === "Q") {
      if (mode === Mode.Clear || mode === Mode.Dead || mode === Mode.AllClear) backToSelect();
    }
  });

  window.addEventListener("keyup", (e) => {
    if (e.code === "Space") {
      e.preventDefault();
      setSpace(false);
    }
  });

  el.cover.addEventListener("click", () => {
    if (mode === Mode.Cover) {
      mode = Mode.Select;
      showScreen("select");
      refreshChrome();
    }
  });

  const hold = el.holdBtn;
  let holdPointerId = null;
  hold.addEventListener("pointerdown", (e) => {
    if (e.button !== undefined && e.button !== 0) return;
    e.preventDefault();
    holdPointerId = e.pointerId;
    // 不用 setPointerCapture：部分浏览器会同步布局卡一下
    setSpace(true);
  });
  function endHold(e) {
    if (holdPointerId === null) return;
    if (e && e.pointerId !== undefined && e.pointerId !== holdPointerId) return;
    holdPointerId = null;
    setSpace(false);
  }
  hold.addEventListener("pointerup", endHold);
  hold.addEventListener("pointercancel", endHold);
  window.addEventListener("pointerup", endHold);
  window.addEventListener("pointercancel", endHold);
  hold.addEventListener("contextmenu", (e) => e.preventDefault());

  window.addEventListener("blur", () => {
    holdPointerId = null;
    setSpace(false);
  });
  window.addEventListener("resize", () => {
    measureArena();
    if (arenaNodes) {
      arenaNodes.tiger._pct = undefined;
      arenaNodes.player._pct = undefined;
      arenaNodes.boat._pct = undefined;
      arenaNodes.hit._pct = undefined;
      if (mode === Mode.Playing || mode === Mode.Clear || mode === Mode.Dead || mode === Mode.AllClear) {
        renderArena();
      }
    }
  });

  el.btnZh.addEventListener("click", () => { locale = "zh"; refreshChrome(); });
  el.btnEn.addEventListener("click", () => { locale = "en"; refreshChrome(); });
  el.btnNext.addEventListener("click", () => {
    if (menuArmed && mode === Mode.Clear) startLevel(levelIndex + 1);
  });
  el.btnRetry.addEventListener("click", () => {
    if (!menuArmed) return;
    if (mode === Mode.AllClear) startLevel(0);
    else if (mode === Mode.Clear || mode === Mode.Dead) startLevel(levelIndex);
  });
  el.btnBack.addEventListener("click", () => {
    if (menuArmed) backToSelect();
  });

  // boot
  paintCoverStrip();
  refreshChrome();
  showScreen("cover");
  requestAnimationFrame(frame);
})();
