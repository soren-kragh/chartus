oss << R"EOF(

const keepAwayFromAxis = 12;
const keepAwayFromCursor = 24;
const lineWidth = 1;
const dotSize = 10;
const infoSpacing = 8;
const snapRadius = 15;

////////////////////////////////////////////////////////////////////////////////

const SVG_NS = "http://www.w3.org/2000/svg";
const svg_cursor = document.getElementById("svgCursor");
const svg_snap = document.getElementById("svgSnap");

let chart;

///////////////////////////////////////////////////////////////////////////////

let cursorTimer;
let cursorHidden = false;

const hideCursor = () => {
  document.body.classList.add('hide-cursor');
  cursorHidden = true;
};

const showCursor = () => {
  clearTimeout(cursorTimer);
  if ( cursorHidden ) {
    document.body.classList.remove('hide-cursor');
  }
  cursorHidden = false;
};

let idList = [];

function genId() {
  const id = "i" + idList.length;
  idList.push(id);
  return id;
}

function removeAll() {
  idList.forEach(id => {
    let existing = svg_cursor.getElementById(id);
    if (existing) {
      existing.parentNode.removeChild(existing);
    }
  });
  idList.length = 0;
}

function newObj(type, addId = false)
{
  const obj = document.createElementNS(SVG_NS, type);
  if (addId) obj.setAttribute("id", genId());
  return obj;
}

////////////////////////////////////////////////////////////////////////////////

const snapKeyFactor = 0.5 / snapRadius;

const coorToSnapKey = (x, y) => {
  const ix = Math.floor(x * snapKeyFactor);
  const iy = Math.floor(y * snapKeyFactor);
  return (ix + 8e6) * 16e6 + (iy + 8e6);
};

function snapMapAdd(snapMap, x, y, snapIdx) {
  const key = coorToSnapKey(x, y);
  let list = snapMap.get(key);
  if (!list) {
    list = [];
    snapMap.set(key, list);
  }
  list.push(snapIdx);
}

function snapMapGet(snapMap, x, y) {
  const cx = Math.round(x * snapKeyFactor) / snapKeyFactor;
  const cy = Math.round(y * snapKeyFactor) / snapKeyFactor;

  const lx = [cx - snapRadius, cx + snapRadius];
  const ly = [cy - snapRadius, cy + snapRadius];

  let minIdx = -1;
  let minDist = snapRadius * snapRadius;

  lx.forEach(bx => {
    ly.forEach(by => {
      const key = coorToSnapKey(bx, by);
      const list = snapMap.get(key);
      if (list) {
        list.forEach(snapIdx => {
          const sp = chart.snapPoints[snapIdx];
          const dx = sp.X - x;
          const dy = sp.Y - y;
          const dist = dx * dx + dy * dy;
          if (dist <= minDist) {
            minIdx = snapIdx;
            minDist = dist;
          }
        });
      }
    });
  });

  return minIdx;
}

////////////////////////////////////////////////////////////////////////////////

function getLinAxisValue(x0, x1, x2, v1, v2) {
  if (x1 === x2) return NaN;
  const t = (x0 - x1) / (x2 - x1);
  return v1 + t * (v2 - v1);
}

function getLogAxisValue(x0, x1, x2, v1, v2) {
  if (x1 === x2 || v1 <= 0 || v2 <= 0) return NaN;
  const t = (x0 - x1) / (x2 - x1);
  return v1 * Math.pow(v2 / v1, t);
}

function coorToVal(coor, axis) {
  let val;
  if ( axis.logarithmic ) {
    val =
      getLogAxisValue(
        coor, axis.coor1, axis.coor2, axis.areaVal1, axis.areaVal2
      );
  } else {
    val =
      getLinAxisValue(
        coor, axis.coor1, axis.coor2, axis.areaVal1, axis.areaVal2
      );
  }
  return val;
}


//------------------------------------------------------------------------------

function getCatIdx(coor, axis, catValues) {
  const min = 0;
  const max = catValues.length - 1;
  if (min > max) return 0;
  let i = Math.round(coorToVal(coor, axis));
  i = Math.min(Math.max(i, 0), chart.catCnt - 1);
  let j1 = min;
  let j2 = max;
  while (j1 + 1 < j2) {
    let i1 = catValues[j1];
    let jm = Math.floor((j1 + j2) / 2);
    let im = catValues[jm];
    if (im < i) {
      j1 = jm;
    } else {
      j2 = jm;
    }
  }
  let candidates = [j1-1, j1, j2, j2+1];
  let bestDist = Infinity;
  candidates.forEach(j => {
    if (j >= min && j <= max) {
      const k = catValues[j];
      const dist =
        Math.abs(
          coor -
          getLinAxisValue(k, axis.areaVal1, axis.areaVal2, axis.coor1, axis.coor2)
        );
      if (dist < bestDist) {
        bestDist = dist;
        i = k;
      }
    }
  });
  return i;
}

//------------------------------------------------------------------------------

function determineDecimals( axis ) {
  if (!axis.show || axis.isCategory) return;
  axis.rounding = 0;
  if (axis.format === "Fixed") {
    const max = Math.max(Math.abs(axis.areaVal1), Math.abs(axis.areaVal2));
    if (max > 1) {
      axis.rounding = Math.pow(10, Math.ceil(Math.log10(max)));
      if (axis.rounding > 1e15) axis.rounding = 1e15;
    }
  }
  axis.decimals = 0;
  while (axis.decimals < 8) {
    let numTxtPrev;
    let numTxt;
    let ok = true;
    let okCnt = Infinity;
    for (let coor = axis.coor1; coor <= axis.coor2; coor++) {
      let num = coorToVal(coor, axis);
      if (axis.rounding) {
        num = Math.round( num / axis.rounding ) * axis.rounding;
      }
      if (axis.format === "Fixed") {
        numTxt = num.toFixed(axis.decimals);
      } else {
        numTxt = num.toExponential(axis.decimals);
      }
      if (numTxt === numTxtPrev) {
        if (okCnt < 10) {
          ok = false;
          break;
        }
        okCnt = 0;
      } else {
        okCnt++;
      }
      numTxtPrev = numTxt;
    }
    if (ok) break;
    if (axis.rounding) {
      axis.rounding = axis.rounding / 10;
      if (axis.rounding < 10) axis.rounding = 0;
    } else {
      axis.decimals++;
    }
  }
}

function toEngineering(num, decimals) {
  let shift = 0;
  let coefS;
  let expS;
  while (true) {
    let toExpStr = num.toExponential(decimals);
    [coefS, expS] = toExpStr.split('e');
    let exp = Number(expS);
    if (exp % 3 === 0) break;
    num = num / 10;
    shift++;
  }
  let [s1, s2] = coefS.split('.');
  s2 = s2 || "";
  while (shift > 0) {
    if (s2) {
      s1 = s1 + s2[0];
      s2 = s2.slice(1);
    } else {
      s1 = s1 + "0";
    }
    shift--;
  }
  coefS = s2 === "" ? s1 : s1 + "." + s2;
  return coefS + "e" + expS;
}

function buildAxisLabel(coor, axis) {
  let group = newObj("g", true);
  group.setAttribute("fill", chart.axisBoxLineColor);
  group.setAttribute("stroke", "none");
  group.setAttribute("font-size", chart.axisFontSize);
  svg_cursor.appendChild(group);

  let val = coorToVal(coor, axis);

  if (axis.isCategory) {
    let i = Math.round(val);
    let txt = chart.catMapToText.get(i);
    if (!txt && chart.catTxtValues.length > 0) {
      i = getCatIdx(coor, axis, chart.catTxtValues);
      const cat_coor =
        getLinAxisValue(i, axis.areaVal1, axis.areaVal2, axis.coor1, axis.coor2);
      if (Math.abs(coor - cat_coor) < 1.25) txt = chart.catMapToText.get(i);
    }
    if (txt) {
      const text = newObj("text");
      text.textContent = txt;
      group.appendChild(text);
    } else {
      svg_cursor.removeChild(group);
      return null;
    }
  } else {
    if (axis.rounding) {
      val = Math.round( val / axis.rounding ) * axis.rounding;
    }
    let valTxt;
    if (axis.format === "Engineering") {
      valTxt = toEngineering(val, axis.decimals);
    } else
    if (axis.format === "Scientific") {
      valTxt = val.toExponential(axis.decimals);
    } else {
      valTxt = val.toFixed(axis.decimals);
    }
    const hasDigits = /[1-9]/.test(valTxt);
    if (hasDigits) {
      if (val >= 0 && axis.showSign) {
        valTxt = '+' + valTxt;
      }
    } else {
      valTxt = "0";
    }
    const parts = valTxt.split(/e/i);
    if (parts.length === 2) {
      const [base, exp] = parts;
      const baseElem = newObj("text");
      baseElem.textContent = base + "×10";
      group.appendChild(baseElem);
      let bbox = group.getBBox();
      const expElem = newObj("text");
      expElem.setAttribute("font-size", chart.axisFontSize * 0.8);
      expElem.setAttribute("x", bbox.x + bbox.width);
      expElem.setAttribute("y", bbox.y + chart.axisFontSize * 0.5);
      expElem.textContent = exp;
      group.appendChild(expElem);
    } else {
      const text = newObj("text");
      text.textContent = valTxt;
      group.appendChild(text);
    }
  }

  let bbox = group.getBBox();

  const padX = chart.axisFontSize / 2;
  const padY = chart.axisFontSize / 4;

  let rect = newObj("rect");
  rect.setAttribute("x", bbox.x - padX);
  rect.setAttribute("y", bbox.y - padY);
  rect.setAttribute("width", bbox.width + 2 * padX);
  rect.setAttribute("height", bbox.height + 2 * padY);
  rect.setAttribute("rx", padX);
  rect.setAttribute("fill", chart.axisBoxFillColor);
  rect.setAttribute("stroke-width", lineWidth)
  rect.setAttribute("stroke", chart.axisBoxLineColor);
  group.insertBefore(rect, group.firstChild);

  return group;
}

////////////////////////////////////////////////////////////////////////////////

function createDot(g1, g2, x, y) {
  let dot = newObj("circle");
  dot.setAttribute("cx", x);
  dot.setAttribute("cy", y);
  dot.setAttribute("r", dotSize/2);
  g1.appendChild(dot);

  dot = newObj("circle");
  dot.setAttribute("cx", x);
  dot.setAttribute("cy", y);
  dot.setAttribute("r", dotSize/2 - 2*lineWidth);
  g2.appendChild(dot);
}

function createCrosshair(x, y, atPoint) {
  let group = newObj("g", true);
  group.setAttribute("stroke-width", lineWidth);
  group.setAttribute("stroke-dasharray", "12 8");
  group.setAttribute("stroke", chart.crosshairLineColor);
  group.setAttribute("fill", chart.crosshairFillColor);

  {
    let line = newObj("line");
    line.setAttribute("x1", x);
    line.setAttribute("y1", y);
    line.setAttribute("x2", x);
    line.setAttribute("y2", chart.area.y2);
    group.appendChild(line);
  }
  {
    let line = newObj("line");
    line.setAttribute("x1", x);
    line.setAttribute("y1", y);
    line.setAttribute("x2", x);
    line.setAttribute("y2", chart.area.y1);
    group.appendChild(line);
  }

  {
    let line = newObj("line");
    line.setAttribute("x1", x);
    line.setAttribute("y1", y);
    line.setAttribute("x2", chart.area.x2);
    line.setAttribute("y2", y);
    group.appendChild(line);
  }
  {
    let line = newObj("line");
    line.setAttribute("x1", x);
    line.setAttribute("y1", y);
    line.setAttribute("x2", chart.area.x1);
    line.setAttribute("y2", y);
    group.appendChild(line);
  }

  if (atPoint) {
    let g1 = newObj("g");
    let g2 = newObj("g");
    group.appendChild(g1);
    group.appendChild(g2);
    g1.setAttribute("stroke", "none")
    g1.setAttribute("fill", chart.crosshairLineColor);
    g2.setAttribute("stroke", "none")
    g2.setAttribute("fill", chart.crosshairFileColor);
    createDot(g1, g2, x, y);
    showCursor();
    if ( chart.hideMouseCursor ) {
      cursorTimer = setTimeout(hideCursor, 500);
    }
  } else {
    if ( chart.hideMouseCursor ) {
      hideCursor();
    }
  }

  svg_cursor.appendChild(group);
}

////////////////////////////////////////////////////////////////////////////////

function createAxisBox(x, y, axis, {isMulti = false} = {}) {
  if (!axis.show) return;

  let coor = x;
  if (axis.id === "axisY_0" || axis.id === "axisY_1") {
    coor = y;
  }
  const group = buildAxisLabel(coor, axis);
  if ( !group ) return;

  let bbox = group.getBBox();

  let transX = -bbox.width/2 - bbox.x;
  let transY = -bbox.height/2 - bbox.y;

  let d = isMulti ? keepAwayFromAxis : keepAwayFromCursor;

  let space = keepAwayFromAxis + d;

  switch (axis.id) {
    case "axisX_0":
      transX += x;
      if ( y - chart.area.y1 < bbox.height + space ) {
        transY += y - bbox.height/2 - d;
      } else {
        transY += chart.area.y1 + bbox.height/2 + keepAwayFromAxis;
      }
      break;
    case "axisX_1":
      transX += x;
      if ( chart.area.y2 - y < bbox.height + space ) {
        transY += y + bbox.height/2 + d;
      } else {
        transY += chart.area.y2 - bbox.height/2 - keepAwayFromAxis;
      }
      break;
    case "axisY_0":
      if ( x - chart.area.x1 < bbox.width + space ) {
        transX += x - bbox.width/2 - d;
      } else {
        transX += chart.area.x1 + bbox.width/2 + keepAwayFromAxis;
      }
      transY += y;
      break;
    case "axisY_1":
      if ( chart.area.x2 - x < bbox.width + space ) {
        transX += x + bbox.width/2 + d;
      } else {
        transX += chart.area.x2 - bbox.width/2 - keepAwayFromAxis;
      }
      transY += y;
      break;
  }

  group.setAttribute("transform", `translate(${transX}, ${transY})`);

  return {
    cx: bbox.x + transX + bbox.width / 2,
    cy: bbox.y + transY + bbox.height / 2,
    wx: bbox.width,
    wy: bbox.height,
    group, transX, transY
  };
}

////////////////////////////////////////////////////////////////////////////////

function resolveOverlaps(boxes, sx, sy) {
  let x1 = (sx > 0) ? (chart.area.x1 + infoSpacing) : -1e12;
  let x2 = (sx < 0) ? (chart.area.x2 - infoSpacing) : +1e12;
  let y1 = (sy > 0) ? (chart.area.y1 + infoSpacing) : -1e12;
  let y2 = (sy < 0) ? (chart.area.y2 - infoSpacing) : +1e12;

  const epsilon = 0.3;
  const maxIter = 3000;
  let iter = 0;
  let overlapping = true;

  while (overlapping && iter < maxIter) {
    overlapping = false;
    let forces = boxes.map(() => ({ fx: 0, fy: 0 }));

    for (let i = 0; i < boxes.length; i++) {
      for (let j = i + 1; j < boxes.length; j++) {
        const a = boxes[i];
        const b = boxes[j];
        if (j > i + 2) break;

        let dx = b.cx - a.cx;
        let dy = b.cy - a.cy;

        let overlapX = (a.wx + b.wx)/2 + infoSpacing - Math.abs(dx);
        let overlapY = (a.wy + b.wy)/2 + infoSpacing - Math.abs(dy);

        if (overlapX > 0 && overlapY > 0) {
          overlapping = true;
          if (sx != 0) {
            const f = sx * (overlapX + 1);
            forces[i].fx -= f;
            forces[j].fx += f;
          }
          if (sy != 0) {
            const f = sy * (overlapY + 1);
            forces[i].fy -= f;
            forces[j].fy += f;
          }
        }
      }
    }

    let md = 0;
    boxes.forEach((box, i) => {
      if (!box.group) return;
      let ox = box.cx;
      let oy = box.cy;
      box.cx += epsilon * forces[i].fx;
      box.cy += epsilon * forces[i].fy;
      box.cx = Math.min(Math.max(box.cx, x1 + box.wx / 2), x2 - box.wx / 2);
      box.cy = Math.min(Math.max(box.cy, y1 + box.wy / 2), y2 - box.wy / 2);
      md = Math.max(md, Math.max(Math.abs(box.cx - ox), Math.abs(box.cy - oy)));
    });

    if (md < 0.1) break;

    iter++;
  }
}

//------------------------------------------------------------------------------

function createCategoryBoxes(x, y, axis) {
  showCursor();

  if (chart.catBoxValues.length == 0) return;

  let horizontal = false;
  let coor = x;
  if (axis.id === "axisY_0" || axis.id === "axisY_1") {
    coor = y;
    horizontal = true;
  }
  let sx = horizontal ? (axis.id == "axisY_0" ? 1 : -1) : 0;
  let sy = horizontal ? 0 : (axis.id == "axisX_0" ? 1 : -1);

  let i = getCatIdx(coor, axis, chart.catBoxValues);

  let snapped_coor =
    getLinAxisValue(i, axis.areaVal1, axis.areaVal2, axis.coor1, axis.coor2);

  const snapIdxList = chart.catMapToSnap.get(i);
  let lst = [];
  if (snapIdxList) {
    snapIdxList.forEach(snapIdx => {
      const sp = chart.snapPoints[ snapIdx ];
      lst.push({serIdx : sp.s, snapIdx : snapIdx, x : sp.X, y : sp.Y});
    });
  }
  if (lst.length == 0) return;
  if (sx > 0 || sy > 0) {
    lst.sort((a, b) => horizontal ? (a.x - b.x) : (a.y - b.y));
  } else {
    lst.sort((a, b) => horizontal ? (b.x - a.x) : (b.y - a.y));
  }

  let group = newObj("g", true);
  group.setAttribute("stroke-width", lineWidth);
  group.setAttribute("stroke", chart.crosshairLineColor);
  group.setAttribute("fill", chart.crosshairFillColor);

  if (chart.inLine) {
    let line = newObj("line");
    line.setAttribute("stroke-dasharray", "12 8");
    if (sx != 0) {
      line.setAttribute("y1", snapped_coor);
      line.setAttribute("y2", snapped_coor);
      line.setAttribute("x1", (sx > 0) ? chart.area.x1 : chart.area.x2);
      line.setAttribute("x2", (sx > 0) ? chart.area.x2 : chart.area.x1);
    } else {
      line.setAttribute("x1", snapped_coor);
      line.setAttribute("x2", snapped_coor);
      line.setAttribute("y1", (sy > 0) ? chart.area.y1 : chart.area.y2);
      line.setAttribute("y2", (sy > 0) ? chart.area.y2 : chart.area.y1);
    }
    group.appendChild(line);
  } else {
    let line_g = newObj("g");
    line_g.setAttribute("stroke-dasharray", "12 8");
    lst.forEach(e => {
      let line = newObj("line");
      let series = chart.seriesList[e.serIdx];
      let x = e.x;
      let y = e.y;
      if (horizontal) {
        if (series?.axisX != undefined) {
          y = (series.axisX == 0) ? chart.area.y1 : chart.area.y2;
        }
      } else {
        if (series?.axisY != undefined) {
          x = (series.axisY == 0) ? chart.area.x1 : chart.area.x2;
        }
      }
      line.setAttribute("x1", x);
      line.setAttribute("y1", y);
      line.setAttribute("x2", e.x);
      line.setAttribute("y2", e.y);
      line_g.appendChild(line);
    });
    group.appendChild(line_g);
  }

  let g1 = newObj("g");
  let g2 = newObj("g");
  group.appendChild(g1);
  group.appendChild(g2);
  g1.setAttribute("stroke", "none")
  g1.setAttribute("fill", chart.crosshairLineColor);
  g2.setAttribute("stroke", "none")
  g2.setAttribute("fill", chart.crosshairFileColor);
  lst.forEach(e => {
    createDot(g1, g2, e.x, e.y);
  });

  svg_cursor.appendChild(group);

  let boxes = [];

  {
    const res = createAxisBox(
      (sx != 0) ? lst[0].x : snapped_coor,
      (sx != 0) ? snapped_coor : lst[0].y,
      axis, {isMulti: true}
    );
    if (res) {
      boxes.push({
        cx: res.cx, cy: res.cy,
        wx: res.wx, wy: res.wy,
      });
    }
  }

  let hx = snapped_coor > (chart.area.x2 + chart.area.x1) / 2;
  let hy = snapped_coor > (chart.area.y2 + chart.area.y1) / 2;
  {
    let anchor = {anchorX: 0, anchorY: 0};
    if (horizontal) {
      anchor.anchorY = hy ? +1 : -1;
    } else {
      anchor.anchorX = hx ? +1 : -1;
    }
    lst.forEach(e => {
      const snapPoint = chart.snapPoints[e.snapIdx];
      const res = createInfoBox(snapPoint, e.x, e.y, anchor, false);
      boxes.push({
        cx: res.cx, cy: res.cy,
        ox: res.cx, oy: res.cy,
        wx: res.wx, wy: res.wy,
        group: res.group,
        transX: res.transX,
        transY: res.transY
      });
    });
    resolveOverlaps(boxes, sx, sy);
    boxes.forEach(box => {
      if (!box.group) return;
      let tx = box.transX + (box.cx - box.ox);
      let ty = box.transY + (box.cy - box.oy);
      box.group.setAttribute("transform", `translate(${tx}, ${ty})`);
    });
  }
}

////////////////////////////////////////////////////////////////////////////////

function createSeriesHighlight(series) {
  if (series?.legendBB == undefined) return;

  const bb = series.legendBB;

  const defs = svg_cursor.querySelector("defs") || newObj("defs");
  if (!svg_cursor.querySelector("defs")) {
    svg_cursor.appendChild(defs);
  }

  const filterId = `hazyEdge-${bb.x1}-${bb.y1}`;
  let filter = document.getElementById(filterId);
  if (!filter) {
    filter = newObj("filter");
    filter.setAttribute("id", filterId);
    filter.setAttribute("filterUnits", "userSpaceOnUse");
    filter.setAttribute("x", bb.x1 - 20);
    filter.setAttribute("y", bb.y1 - 20);
    filter.setAttribute("width", bb.x2 - bb.x1 + 40);
    filter.setAttribute("height", bb.y2 - bb.y1 + 40);
    const blur = newObj("feGaussianBlur");
    blur.setAttribute("in", "SourceGraphic");
    blur.setAttribute("stdDeviation", "10");
    filter.appendChild(blur);
    defs.appendChild(filter);
  }

  const rect = newObj("rect", true);
  rect.setAttribute("x", bb.x1);
  rect.setAttribute("y", bb.y1);
  rect.setAttribute("width", bb.x2 - bb.x1);
  rect.setAttribute("height", bb.y2 - bb.y1);
  rect.setAttribute("rx", chart.axisFontSize);
  rect.setAttribute("stroke", "none");
  rect.setAttribute("fill", chart.highlightColor);
  rect.setAttribute("filter", `url(#${filterId})`);
  rect.setAttribute("opacity", "0.3");
  svg_cursor.appendChild(rect);
}

////////////////////////////////////////////////////////////////////////////////

function createInfoBox(snapPoint, x, y, anchor, highlightSeries = true) {
  let series = chart.seriesList[snapPoint.s]

  if (highlightSeries) createSeriesHighlight(series);

  // Create a group (<g>) to hold the box and text
  const group = newObj("g", true);
  group.setAttribute("font-size", chart.infoFontSize);
  group.setAttribute("font-weight", "bold")
  group.setAttribute("fill", series.txColor);
  group.setAttribute("stroke", "none");

  const padX = chart.infoFontSize / 2;
  const padY = chart.infoFontSize / 4;

  let textualX = false;
  let itemX = snapPoint.x;
  let itemY = snapPoint.y;
  if (typeof itemX === "number") {
    textualX = true;
  }

  {
    let text = newObj("text");
    if ( textualX ) {
      text.textContent = itemY;
    } else {
      text.textContent = "(" + itemX + "," + itemY + ")";
    }
    group.appendChild(text);
  }

  svg_cursor.appendChild(group);
  let bbox = group.getBBox();

  {
    let rect = newObj("rect");
    rect.setAttribute("x", bbox.x - padX);
    rect.setAttribute("y", bbox.y - padY);
    rect.setAttribute("width", bbox.width + 2 * padX);
    rect.setAttribute("height", bbox.height + 2 * padY);
    rect.setAttribute("rx", padX);
    rect.setAttribute("fill", series.bgColor);
    rect.setAttribute("stroke-width", 2 * lineWidth)
    rect.setAttribute("stroke", series.fgColor);
    group.insertBefore(rect, group.firstChild);
  }

  bbox = group.getBBox();

  let transX = x - bbox.x - bbox.width / 2;
  let transY = y - bbox.y - bbox.height / 2;
  transX -= anchor.anchorX * (bbox.width / 2 + keepAwayFromAxis);
  transY -= anchor.anchorY * (bbox.height / 2 + keepAwayFromAxis);

  group.setAttribute("transform", `translate(${transX}, ${transY})`);

  return {
    cx: bbox.x + transX + bbox.width / 2,
    cy: bbox.y + transY + bbox.height / 2,
    wx: bbox.width,
    wy: bbox.height,
    group, transX, transY
  };
}

////////////////////////////////////////////////////////////////////////////////

function outOfArea() {
  removeAll();
  showCursor();
}

function getMouseSVGPoint(event) {
  const pt = svg_snap.createSVGPoint();
  pt.x = event.clientX;
  pt.y = event.clientY;
  return pt.matrixTransform(svg_snap.getScreenCTM().inverse());
}

svg_snap.addEventListener("mousemove", (event) => {
  removeAll();

  const svgPoint = getMouseSVGPoint(event);
  const mouseX = svgPoint.x;
  const mouseY = svgPoint.y;

  chart = undefined;
  let minDist = Infinity;
  for (let i = 0; i < chart_list.length; i++) {
    const c = chart_list[i];
    const { x1, y1, x2, y2 } = c.chart;

    let dx = 0;
    if (mouseX < x1) dx = x1 - mouseX;
    if (mouseX > x2) dx = mouseX - x2;

    let dy = 0;
    if (mouseY < y1) dy = y1 - mouseY;
    if (mouseY > y2) dy = mouseY - y2;

    const dist = Math.sqrt(dx*dx + dy*dy);

    if (dist < minDist) {
      minDist = dist;
      chart = c;
    }
  }

  if (chart != undefined) {
    const inAreaX = mouseX >= chart.area.x1 && mouseX <= chart.area.x2;
    const inAreaY = mouseY >= chart.area.y1 && mouseY <= chart.area.y2;
    const inArea = inAreaX && inAreaY;

    const minCatWidth = 24;
    let inCat = false;
    let catAxis;
    if (chart.axisX[0].isCategory) {
      let y = Math.max( chart.area.y1, chart.chart.y1 + minCatWidth );
      inCat = mouseY <= y;
      catAxis = chart.axisX[0];
    }
    if (chart.axisX[1].isCategory) {
      let y = Math.min( chart.area.y2, chart.chart.y2 - minCatWidth );
      inCat = mouseY >= y;
      catAxis = chart.axisX[1];
    }
    if (chart.axisY[0].isCategory) {
      let x = Math.max( chart.area.x1, chart.chart.x1 + minCatWidth );
      inCat = mouseX <= x;
      catAxis = chart.axisY[0];
    }
    if (chart.axisY[1].isCategory) {
      let x = Math.min( chart.area.x2, chart.chart.x2 - minCatWidth );
      inCat = mouseX >= x;
      catAxis = chart.axisY[1];
    }

    const snapIdx = snapMapGet(chart.snapMap, mouseX, mouseY);

    if (snapIdx >= 0 || inArea || inCat) {
      let x = mouseX;
      let y = mouseY;
      let snapPoint;
      let atPoint = false;

      if (snapIdx >= 0) {
        snapPoint = chart.snapPoints[ snapIdx ];
        x = snapPoint.X;
        y = snapPoint.Y;
        atPoint = true;
        inCat = false;
      }

      if (inCat) {
        createCategoryBoxes(x, y, catAxis);
      } else {
        createCrosshair(x, y, atPoint);
        let showX = [true, true];
        let showY = [true, true];
        if (atPoint) {
          showX = [false, false];
          showY = [false, false];
          let series = chart.seriesList[snapPoint.s];
          if (series?.axisX != undefined) showX[series.axisX] = true;
          if (series?.axisY != undefined) showY[series.axisY] = true;
        }
        if (showX[0]) createAxisBox(x, y, chart.axisX[0]);
        if (showX[1]) createAxisBox(x, y, chart.axisX[1]);
        if (showY[0]) createAxisBox(x, y, chart.axisY[0]);
        if (showY[1]) createAxisBox(x, y, chart.axisY[1]);
        if (atPoint) {
          let anchor = {anchorX: -1, anchorY: -1};
          if (x > (chart.area.x2 + chart.area.x1)/2) anchor.anchorX = 1;
          if (y > (chart.area.y2 + chart.area.y1)/2) anchor.anchorY = 1;
          createInfoBox(snapPoint, x, y, anchor);
        }
      }
    } else {
      chart = undefined;
    }
  }

  if (chart == undefined) {
    outOfArea();
  }
});

svg_snap.addEventListener("mouseleave", () => {
  outOfArea();
});

////////////////////////////////////////////////////////////////////////////////
// Main.

{
  chart_list.forEach(c => {
    chart = c;

    chart.axisX[0].id = "axisX_0";
    chart.axisX[1].id = "axisX_1";
    chart.axisY[0].id = "axisY_0";
    chart.axisY[1].id = "axisY_1";
    chart.axisX.forEach(axis => {
      axis.coor1 = chart.area.x1;
      axis.coor2 = chart.area.x2;
      determineDecimals( axis );
    });
    chart.axisY.forEach(axis => {
      axis.coor1 = chart.area.y1;
      axis.coor2 = chart.area.y2;
      determineDecimals( axis );
    });

    // Create mapping from category X-value to the category text.
    chart.catMapToText = new Map();
    if (chart.catCnt) {
      let i = 0;
      chart.categories.forEach(cat => {
        if (typeof cat === "number") {
          i = cat;
        } else {
          chart.catMapToText.set(i++, cat);
        }
      });
    }

    // Maps a category X-value to a list of associated snap points.
    chart.catMapToSnap = new Map();
    if (chart.catCnt) {
      let snapIdx = 0;
      chart.snapPoints.forEach(sp => {
        if (typeof sp.x === "number") {
          let list = chart.catMapToSnap.get(sp.x);
          if (!list) {
            list = [];
            chart.catMapToSnap.set(sp.x, list);
          }
          list.push(snapIdx);
        }
        snapIdx++;
      });
    }

    // Make a sorted list of category X-values, and a sorted list of category
    // X-values which have at least one associated snap point.
    chart.catTxtValues = [];
    chart.catBoxValues = [];
    for (let i = 0; i < chart.catCnt; i++) {
      if (chart.catMapToText.has(i)) chart.catTxtValues.push(i);
      if (chart.catMapToSnap.has(i)) chart.catBoxValues.push(i);
    }

    // Add all snap points to spatial map.
    {
      chart.snapMap = new Map();
      let snapIdx = 0;
      chart.snapPoints.forEach(sp => {
        snapMapAdd(chart.snapMap, sp.X, sp.Y, snapIdx);
        snapIdx++;
      });
    }

  });
}

////////////////////////////////////////////////////////////////////////////////
</script>

</body>
</html>

)EOF";
